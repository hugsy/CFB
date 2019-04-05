using System;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;
using System.ComponentModel;

namespace Fuzzer
{
    public partial class IrpMonitorForm : Form
    {
        private IrpDataReader DataReader;
        private GlobalSettingsForm SettingsForm;
        private LoadDriverForm DriverForm;
        public IrpFilterForm FilterForm;
        private static Mutex LogMutex;
        private bool bIsDriverLoaded;
        private bool bIsMonitoringEnabled;
        private bool bRunAsPrivileged;



        public IrpMonitorForm()
        {
            InitializeComponent();
            SetLoadedDriverStatus(false);
            SetMonitoringStatus(false);

            LogMutex = new Mutex();
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(OnProcessExit);
            DataReader = new IrpDataReader(this);
            DriverForm = new LoadDriverForm(this);
            SettingsForm = new GlobalSettingsForm(this);
            FilterForm = new IrpFilterForm(this, DataReader);
        }


        public void Log(string message)
        {
            var t = new Thread(() =>
            {
                LogMutex.WaitOne();
                LogTextBox.AppendText( $"{message:s}\r\n" );
                LogMutex.ReleaseMutex();
            });
            t.Start();
            t.Join();
        }


        public void LogErr(string title)
        {
            var gle = Kernel32.GetLastError();
            var ex = new Win32Exception((int)gle);
            var text = $"{title} \r\n\r\n {ex.Message}";
            Log(text);
        }


        private static void PopupError(string v, bool bPrintLastError = true)
        {
            string text;
            string title;

            if (bPrintLastError)
            {
                var gle = Kernel32.GetLastError();
                var ex = new Win32Exception((int)gle);
                text = $"{v} \r\n\r\n {ex.Message}";
                title = $"CFB initialization failed (Error={gle})";
            }
            else
            {
                text = v;
                title = "Error !";
            }

            MessageBox.Show(
                text,
                title,
                MessageBoxButtons.OK,
                MessageBoxIcon.Error,
                MessageBoxDefaultButton.Button1
            );
        }


        private void StartListening()
        {
            DataReader.StartThreads();
        }

        private void StopListening()
        {
            DataReader.JoinThreads();
        }

        private void OnProcessExit(object sender, EventArgs e)
        {
            Core.DisableMonitoring();

            if (DataReader.AreThreadsRunning )
                StopListening();

            CleanupCfbContext();
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            InitCfbContext();
        }


        private void InitCfbContext()
        {
            if (bIsDriverLoaded)
                return;

            Log("Initializing CFB driver...");

            try
            {
                Core.CheckWindowsVersion();
                Core.RunInitializationChecks();
                Core.LoadDriver();
                Core.OpenDeviceHandle();
                Log("Successful initialization, hook some drivers and start monitoring!");
                SetLoadedDriverStatus(true);
                bRunAsPrivileged = true;
                return;
            }
            catch(CoreInitializationException Ex)
            {
                bRunAsPrivileged = false;
                PopupError(Ex.Message);
                Application.Exit();
            }

            Log("Failed to initialize the driver, CFB will run as unprivileged");
            DisablePrivilegedActions();
            return;
        }


        private void Form1_Unload(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }


        private void CleanupCfbContext()
        {
            if (!bRunAsPrivileged)
                return;

            Log("Cleaning up context (may take a bit, be patient)...");

            try
            {
                if (bIsDriverLoaded)
                {
                    Core.CloseDeviceHandle();
                    Core.UnloadDriver();
                    Log("Success...");
                    SetLoadedDriverStatus(false);
                    SetMonitoringStatus(false);
                }
            }
            catch (CoreInitializationException Ex)
            {               
                LogErr(Ex.Message);
                Log("An error occured during context cleanup. It is recommended to close the application.");
            }
        }


        private void QuitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult dialogResult = MessageBox.Show("Are you sure you want to leave CFB ?", "Leave CFB", MessageBoxButtons.YesNo);
            if (dialogResult == DialogResult.Yes)
            {
                StopListening();
                CleanupCfbContext();
                Application.Exit();
            }
        }

        private void ShowIrpDetailsForm()
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = DataReader.Irps[SelectedRow.Index];
            IrpViewerForm HexViewForm = new IrpViewerForm(SelectedRow.Index, SelectedIrp);
            HexViewForm.Show();
        }

        private void ShowIrpBtn_Click(object sender, EventArgs e)
        {
            ShowIrpDetailsForm();
        }

        private void IrpDataView_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            ShowIrpDetailsForm();
        }

        private void DumpToFileBtn_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = DataReader.Irps[SelectedRow.Index];
            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = "Raw|*.raw",
                Title = "Save IRP body to file"
            };
            saveFileDialog.ShowDialog();

            if (saveFileDialog.FileName != "")
            {
                System.IO.FileStream fs = (System.IO.FileStream)saveFileDialog.OpenFile();
                fs.Write(SelectedIrp.Body, 0, SelectedIrp.Body.Length);
                fs.Close();
                Log($"Saved as '{saveFileDialog.FileName:s}'");
            }
 
        }


        private void IrpDataView_SelectionChanged(object sender, EventArgs e)
        {
            
            switch (IrpDataView.SelectedRows.Count)
            {              
                case 1:
                    ShowIrpBtn.Enabled = true;
                    DumpToFileBtn.Enabled = true;
                    SaveForReplayPyBtn.Enabled = true;
                    SaveForReplayPsBtn.Enabled = true;

                    Irp SelectedIrp = DataReader.Irps[IrpDataView.SelectedRows[0].Index];

                    if ((Irp.IrpMajorType)SelectedIrp.Header.Type == Irp.IrpMajorType.IRP_MJ_DEVICE_CONTROL)
                    {
                        FuzzIrpBtn.Enabled = true;
                    }
                    
                    break;

                default:
                    ShowIrpBtn.Enabled = false;
                    DumpToFileBtn.Enabled = false;
                    SaveForReplayPyBtn.Enabled = false;
                    SaveForReplayPsBtn.Enabled = false;
                    FuzzIrpBtn.Enabled = false;
                    break;
            }

            return;
        }


        private void SaveForReplayPsBtn_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = DataReader.Irps[SelectedRow.Index];

            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = "PowerShell|*.ps1",
                Title = "Save IRP body to file"
            };

            if (saveFileDialog.ShowDialog() == DialogResult.OK && saveFileDialog.FileName != "")
            {
                string IrpDataInStr = "[Byte[]] @(" + Environment.NewLine;

                int idx = 0;
                foreach (byte c in SelectedIrp.Body)
                {
                    if (idx != 0 && idx % 16 == 0)
                    {
                        IrpDataInStr += Environment.NewLine;
                    }
                    IrpDataInStr += $"0x{c:X2}";

                    if (idx != SelectedIrp.Body.Length - 1)
                        IrpDataInStr += ", ";

                    idx ++;
                }
                IrpDataInStr += Environment.NewLine + $")";


                string IrpDataOutStr = "";

                if (SelectedIrp.Header.OutputBufferLength > 0)
                {
                    IrpDataOutStr += $"[Byte[]] @(0x00) * {SelectedIrp.Header.OutputBufferLength:d}";
                }
                else
                {
                    IrpDataOutStr += "[Byte[]] @()";
                }

                string DeviceName = SelectedIrp.DeviceName.Replace("\\Device", "\\\\.");

                string PowershellReplayTemplate = String.Join(
                    Environment.NewLine,
                    "#",
                    "#",
                    $"# Replay IOCTL 0x{SelectedIrp.Header.IoctlCode:x} script to {SelectedIrp.DeviceName:s}",
                    "# Auto-generated by CFB",
                    "#",
                    "",
                    "",
                    "# Source: pinvoke.net",
                    "Add-Type -TypeDefinition @\""
                    );

                PowershellReplayTemplate += @"
using System;
using System.Diagnostics;	
using System.Runtime.InteropServices;	
using System.Security.Principal;

public static class CFB
{
    [DllImport(""kernel32.dll"", CharSet = CharSet.Auto, SetLastError = true)]
    public static extern IntPtr CreateFile(
        String lpFileName,
        UInt32 dwDesiredAccess,
        UInt32 dwShareMode,
        IntPtr lpSecurityAttributes,
        UInt32 dwCreationDisposition,
        UInt32 dwFlagsAndAttributes,
        IntPtr hTemplateFile
    );
    [DllImport(""kernel32.dll"", SetLastError = true)]
    public static extern bool DeviceIoControl(
        IntPtr hDevice,
        int IoControlCode,
        byte[] InBuffer,
        int nInBufferSize,
        byte[] OutBuffer,
        int nOutBufferSize,
        ref int pBytesReturned,
        IntPtr Overlapped
    );
    [DllImport(""kernel32.dll"", SetLastError = true, CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Auto)]
    public static extern bool CloseHandle(IntPtr hObject);
    [DllImport(""kernel32.dll"", SetLastError = true)]
    public static extern void OutputDebugString(string lpOutputString);
    [DllImport(""kernel32.dll"")]
    public static extern uint GetLastError();
}";
                PowershellReplayTemplate += Environment.NewLine + "\"@";

                PowershellReplayTemplate += Environment.NewLine + $@"
function Debug-Info
{{
    param([string]$Message )
    [CFB]::OutputDebugString(""[*] "" + $Message + ""`n"")
    Write-Host(""[*] "" + $Message)
}}

function Debug-Success
{{
    param([string]$Message )
    [CFB]::OutputDebugString(""[+] "" + $Message + ""`n"")
    Write-Host(""[+] "" + $Message)
}}

function Debug-Error
{{
    param([string]$Message )
    [CFB]::OutputDebugString(""[-] "" + $Message + ""`n"")
    Write-Error(""[-] "" + $Message)
}}

$IrpDataIn = {IrpDataInStr:s}
$IrpDataOut = {IrpDataOutStr:s}
$IoctlCode = 0x{SelectedIrp.Header.IoctlCode:x}
$DeviceName = ""{DeviceName:s}""

Debug-Info(""Getting handle to driver..."")

Set-Variable OPEN_EXISTING -Option Constant -Value 3
Set-Variable FILE_ATTRIBUTE_NORMAL -Option Constant -Value 0x80

$hDevice = [CFB]::CreateFile(
    $DeviceName,
    [System.IO.FileAccess]::ReadWrite,
    [System.IO.FileShare]::ReadWrite,
    [System.IntPtr]::Zero,
    $OPEN_EXISTING,
    $FILE_ATTRIBUTE_NORMAL,
    [System.IntPtr]::Zero
)

if ($hDevice -eq -1)
{{
    Debug-Error(""Unable to get driver handle."")
    Return
}}

$dwReturnLength = 0

Debug-Info(""Sending request (IoctlCode=#{{0:x}})..."" -f $IoctlCode)
$res = [CFB]::DeviceIoControl(
    $hDevice,
    $IoctlCode,
    $IrpDataIn,
    $IrpDataIn.Length,
    $IrpDataOut,
    $IrpDataOut.Length,
    [ref]$dwReturnLength,
    [System.IntPtr]::Zero
)

if ( $res -eq $true )
{{
    Debug-Success(""Success"")
    if($dwReturnLength -gt 0)
    {{
        $IrpDataOut | Format-Hex
    }}
}}
else
{{
    Debug-Error(""Failed, GetLastError=#{{0:x}}"" -f [CFB]::GetLastError())
}}

[CFB]::CloseHandle($hDevice)

";
                System.IO.File.WriteAllText(saveFileDialog.FileName, PowershellReplayTemplate);

                Log($"Saved as '{saveFileDialog.FileName:s}'");
            }
        }


        private void SaveForReplayPyBtn_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = DataReader.Irps[SelectedRow.Index]; 
                                 
            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = "Python|*.py",
                Title = "Save IRP body to file"
            };

            if (saveFileDialog.ShowDialog() == DialogResult.OK && saveFileDialog.FileName != "")
            {
                string IrpDataInStr = "";
                string IrpDataOutStr = "''";

                foreach ( byte c in SelectedIrp.Body )
                {
                    IrpDataInStr += $"\\x{c:X2}";
                }

                if (SelectedIrp.Header.OutputBufferLength > 0)
                {
                    IrpDataOutStr += $"b'\\x00'*{SelectedIrp.Header.OutputBufferLength:d}";
                }

                string DeviceName = SelectedIrp.DeviceName.Replace("\\Device", "\\\\.");

                string PythonReplayTemplate = 
$@"'''

Replay IOCTL 0x{SelectedIrp.Header.IoctlCode:x} script to {SelectedIrp.DeviceName:s}
Auto-generated by CFB

'''

import sys
from ctypes import *
from contextlib import contextmanager

try:
    import win32con
except ImportError:
    print('win32 package is required: pip install pywin32')
    sys.exit(1)

ntdll       = windll.ntdll
kernel32    = windll.kernel32
KdPrint     = lambda x:  kernel32.OutputDebugStringA(x + '\n')

GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 3

def Hexdump(src, length=16):
    FILTER = ''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])
    lines = []
    for c in range(0, len(src), length):
        chars = src[c:c+length]
        hex = ' '.join(['%02x' % ord(x) for x in chars])
        printable = ''.join(['%s' % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
        lines.append('%04x  %-*s  %s\n' % (c, length * 3, hex, printable))
    return ''.join(lines)


@contextmanager
def GetDeviceHandle(DeviceName, *args, **kwargs):
    Access = kwargs.get('dwDesiredAccess', GENERIC_READ | GENERIC_WRITE)
    handle = kernel32.CreateFileA(DeviceName, Access, 0, None, OPEN_EXISTING, 0, None)
    if handle == -1: raise IOError('Cannot get handle to %s' % DeviceName)
    try: yield handle
    finally: kernel32.CloseHandle(handle)


def DeviceIoctlControl(DeviceName, IoctlCode, _in='', _out='', *args, **kwargs):
    dwBytesReturned = c_uint32()
    InputBufferSize = kwargs.get('_inlen', len(_in))
    OutputBufferSize = kwargs.get('_outlen', len(_out))
    InputBuffer = create_string_buffer(InputBufferSize)
    OutputBuffer = create_string_buffer(OutputBufferSize)
    InputBuffer.value = _in
    OutputBuffer.value = _out
    res = -1
    with GetDeviceHandle(DeviceName) as hDriver:
        KdPrint('Sending inbuflen=%dB to %s with ioctl=%#x (outbuflen=%dB)' % (InputBufferSize, DeviceName, IoctlCode, OutputBufferSize))
        res = kernel32.DeviceIoControl(hDriver, IoctlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, byref(dwBytesReturned), None)
        KdPrint('Sent %dB to %s with IoctlCode %#x' % (InputBufferSize, DeviceName, IoctlCode ))
        if res:
            if dwBytesReturned: 
                print(Hexdump(OutputBuffer))
        else:
            print( GetLastError(), FormatError(GetLastError()) )
    return res


def Trigger():
    DeviceName = r'''{DeviceName:s}'''
    IoctlCode = 0x{SelectedIrp.Header.IoctlCode:x}
    lpIrpDataIn = b'{IrpDataInStr:s}'
    lpIrpDataOut = {IrpDataOutStr:s}
    return DeviceIoctlControl(DeviceName, IoctlCode, lpIrpDataIn, lpIrpDataOut)


if __name__ == '__main__':
    Trigger()
";
                System.IO.File.WriteAllText(saveFileDialog.FileName, PythonReplayTemplate);

                Log($"Saved as '{saveFileDialog.FileName:s}'");
            }
            
        }


        private void FuzzIrpBtn_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = DataReader.Irps[SelectedRow.Index];
            SimpleFuzzerForm fuzzer = new SimpleFuzzerForm(SelectedIrp);
            fuzzer.Show();
        }


        private void SetLoadedDriverStatus(bool NewValue, bool UpdateGui = true)
        {
            bIsDriverLoaded = NewValue;

            if (!UpdateGui)
                return;

            if (bIsDriverLoaded)
            {
                loadDriverToolStripMenuItem.Enabled = false;
                unloadIrpDumperDriverToolStripMenuItem.Enabled = true;
                monitoringToolStripMenuItem.Enabled = true;

                // Cannot add driver when monitoring is ON
                hookUnhookDriversToolStripMenuItem.Enabled = !bIsMonitoringEnabled;
            }
            else
            {
                loadDriverToolStripMenuItem.Enabled = true;
                unloadIrpDumperDriverToolStripMenuItem.Enabled = false;
                monitoringToolStripMenuItem.Enabled = false;
                hookUnhookDriversToolStripMenuItem.Enabled = false;
            }

            // Refresh monitoring button settings
            SetMonitoringStatus(bIsMonitoringEnabled, UpdateGui);
        }


        private void SetMonitoringStatus(bool NewValue, bool UpdateGui = true)
        {
            bIsMonitoringEnabled = NewValue;

            if (!UpdateGui)
                return;

            if (bIsMonitoringEnabled)
            {
                startMonitoringToolStripMenuItem.Enabled = false;
                stopMonitoringToolStripMenuItem.Enabled = true;
                CleanIrpDataGridButton.Enabled = false;
                StatusBar.Text = $"Monitoring for new IRPs on {DriverForm.LoadedDrivers.Count} driver(s)...";
            }
            else
            {
                startMonitoringToolStripMenuItem.Enabled = true;
                stopMonitoringToolStripMenuItem.Enabled = false;
                CleanIrpDataGridButton.Enabled = true;
                StatusBar.Text = "Not Monitoring";
            }
        }

        private void LoadDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            InitCfbContext();
            SetLoadedDriverStatus(true);
        }


        private void UnloadIrpDumperDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CleanupCfbContext();
            SetLoadedDriverStatus(false, true);
        }

        private void StartMonitoringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Log("Starting monitoring...");
            StartListening();
            Core.EnableMonitoring();
            SetMonitoringStatus(true);
        }


        private void StopMonitoringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Log("Stopping monitoring...");
            Core.DisableMonitoring();
            StopListening();
            SetMonitoringStatus(false);
        }


        private void ByPathToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string DriverName = SimplePromptPopup.ShowDialog(
                "Enter the complete path to the driver object (example '\\driver\\fvevol'):", 
                "Driver full path"
            );

            if (DriverName.Length == 0)
            {
                return;
            }

            DriverName = DriverName.ToLower();

            if (!Core.HookDriver(DriverName))
            {
                Log($"Failed to hook '{DriverName}'");               
                return;
            }

            DriverForm.LoadedDrivers.Add(DriverName);
            Log($"Driver object '{DriverName}' is now hooked.");
        }

        private void FromListToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form fc = Application.OpenForms["LoadDriverForm"];

            if (fc == null || fc.Visible == false)
            {
                DriverForm.RefreshDriverList();
                DriverForm.Show();
            }
        }

        private void CleanIrpDataGridButton_Click(object sender, EventArgs e)
        {
            DataReader.ResetDataBinder();            
        }

        private void LoadIrpDBToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog()
            {
                Filter = "CFB Database|*.csv",
                Title = "Load IRPs from file"
            };

            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                if ( DataReader.PopulateViewFromFile(openFileDialog.FileName) )
                {
                    Log($"IRPs loaded from '{openFileDialog.FileName:s}'");
                }
                else
                {
                    Log("Failed to load IRPs");
                }
            }
        }

        private void SaveIrpToDBToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (DataReader.Irps.Count == 0)
            {
                Log("Nothing to save");
                return;
            }

            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = "CFB Database|*.csv",
                Title = "Save all the IRPs in View to file"
            };

            if (saveFileDialog.ShowDialog() == DialogResult.OK && saveFileDialog.FileName != "")
            {
                if ( DataReader.DumpViewToFile(saveFileDialog.FileName) )
                {
                    Log($"IRPs saved in '{saveFileDialog.FileName}'");
                }
                else
                {
                    Log($"Failed to save IRPs");
                }
            }

        }

        private void DefineFiltersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(!bRunAsPrivileged)
            {
                PopupError("This action is only available in Privileged mode.");
                return;
            }

            Form f = Application.OpenForms["DefineFilterForm"];

            if (f == null || f.Visible == false)
            {
                FilterForm.Show();
            }
        }

        private void SettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form f = Application.OpenForms["GlobalSettingsForm"];

            if (f == null || f.Visible == false)
            {
                SettingsForm.RefreshSettings();
                SettingsForm.Show();
            }
        }


        private void DisablePrivilegedActions()
        {
            SetMonitoringStatus(false, true);
            SetLoadedDriverStatus(false, true);
        }
    }
}
