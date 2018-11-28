using System;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;
using System.ComponentModel;

namespace Fuzzer
{
    public partial class Form1 : Form
    {
        private CfbDataReader CfbReader;
        private LoadDriverForm ldForm;
        private static Mutex LogMutex;
        private bool bIsDriverLoaded;
        private bool bIsMonitoringEnabled;


        public Form1()
        {
            InitializeComponent();
            SetLoadedDriverStatus(false);
            SetMonitoringStatus(false);

            LogMutex = new Mutex();
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(OnProcessExit);
            CfbReader = new CfbDataReader(this);
            ldForm = new LoadDriverForm(this);
        }

        public void Log(string message)
        {
            var t = new Thread(() =>
            {
                LogMutex.WaitOne();
                LogTextBox.AppendText( $"{message:s}\n" );
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


        private static void PopupError(string v)
        {
            var gle = Kernel32.GetLastError();
            var ex = new Win32Exception((int)gle);
            var text = $"{v} \r\n\r\n {ex.Message}";
            MessageBox.Show(
                text,
                $"CFB initialization failed (Error={gle})",
                MessageBoxButtons.OK,
                MessageBoxIcon.Error,
                MessageBoxDefaultButton.Button1
            );
        }


        private void StartListening()
        {
            CfbReader.StartThreads();
        }

        private void StopListening()
        {
            CfbReader.JoinThreads();
        }

        private void OnProcessExit(object sender, EventArgs e)
        {
            Core.DisableMonitoring();

            if (CfbReader.AreThreadsRunning )
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

            Log("Initializing CFB...");

            try
            {
                Core.CheckWindowsVersion();
                Core.RunInitializationChecks();
                Core.LoadDriver();
                Core.OpenDeviceHandle();
            }
            catch(CoreInitializationException Ex)
            {
                PopupError(Ex.Message);
                Application.Exit();
            }

            Log("Successful initialization, hook some drivers and start monitoring...");
            SetLoadedDriverStatus(true);
        }


        private void Form1_Unload(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }


        private void CleanupCfbContext()
        {
            if (!bIsDriverLoaded)
                return;

            Log("Cleaning up context (may take a bit, be patient)...");
            try
            {
                Core.CloseDeviceHandle();
                Core.UnloadDriver();
                Log("Success...");
                SetLoadedDriverStatus(false);
                SetMonitoringStatus(false);
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
            Irp SelectedIrp = CfbReader.Irps[SelectedRow.Index];
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
            Irp SelectedIrp = CfbReader.Irps[SelectedRow.Index];
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
                    SaveForReplayBtn.Enabled = true;
                    FuzzIrpBtn.Enabled = true;
                    break;

                default:
                    ShowIrpBtn.Enabled = false;
                    DumpToFileBtn.Enabled = false;
                    SaveForReplayBtn.Enabled = false;
                    FuzzIrpBtn.Enabled = false;
                    break;
            }

            return;
        }
        

        private void SaveForReplayBtn_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = IrpDataView.SelectedRows[0];
            Irp SelectedIrp = CfbReader.Irps[SelectedRow.Index]; 
                                 
            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = "Python|*.py",
                Title = "Save IRP body to file"
            };
            saveFileDialog.ShowDialog();

            if (saveFileDialog.FileName != "")
            {
                string IrpDataInStr = "";
                string IrpDataOutStr = "";

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
    Access = kwargs.get('dwDesiredAccess', win32con.GENERIC_READ | win32con.GENERIC_WRITE)
    handle = kernel32.CreateFileA(DeviceName, Access, 0, None, win32con.OPEN_EXISTING, 0, None)
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
        if res and dwBytesReturned: print(Hexdump(OutputBuffer))
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
            Irp SelectedIrp = CfbReader.Irps[SelectedRow.Index];
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
                StatusBar.Text = $"Monitoring for new IRPs on {ldForm.LoadedDrivers.Count} drivers...";
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
            string DriverName = SimplePromptPopup.ShowDialog("Enter the complete path to the driver object (example '\\driver\\fvevol'):", "Driver full path");

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

            ldForm.LoadedDrivers.Add(DriverName);
            Log($"Driver object '{DriverName}' is now hooked.");
        }

        private void FromListToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form fc = Application.OpenForms["LoadDriverForm"];

            if (fc == null || fc.Visible == false)
            {
                ldForm.RefreshDriverList();
                ldForm.Show();
            }
        }

        private void CleanIrpDataGridButton_Click(object sender, EventArgs e)
        {
            CfbReader.ResetDataBinder();            
        }
    }
}
