using System;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;

namespace Fuzzer
{
    public partial class Form1 : Form
    {
        private CfbDataReader CfbReader;
        private LoadDriverForm ldForm;
        private static Mutex LogMutex;

        private Boolean bIsDriverLoaded;
        private Boolean bIsMonitoringEnabled;



        public Form1()
        {
            InitializeComponent();
            bIsDriverLoaded = false;
            bIsMonitoringEnabled = false;
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

        private void StartListening()
        {
            CfbReader.StartClientThread();
        }

        private void StopListening()
        {
            CfbReader.EndClientThreads();
        }

        private void OnProcessExit(object sender, EventArgs e)
        {
            Core.DisableMonitoring();

            if (CfbReader.IsThreadRunning)
                StopListening();

            CleanupCfbContext();
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            InitCfbContext();
        }


        private void InitCfbContext()
        {
            Log("Initializing CFB...");

            bIsDriverLoaded = false;
            bIsMonitoringEnabled = false;

            Log("Checking Windows version support...");
            if (!Core.CheckWindowsVersion())
            {
                MessageBox.Show("CheckWindowsVersion() failed");
                Application.Exit();
            }

            Log("Running checks...");
            if (!Core.RunInitializationChecks())
            {
                MessageBox.Show("RunInitializationChecks() failed");
                Application.Exit();
            }

            Log("Loading driver...");
            if (!Core.LoadDriver())
            {
                MessageBox.Show("LoadDriver() failed");
                Application.Exit();
            }

            Log("Initializing CFB context...");
            if (!Core.InitializeCfbContext())
            {
                MessageBox.Show("InitializeCfbContext() failed");
                Core.UnloadDriver();
                Application.Exit();
            }

            bIsDriverLoaded = true;
            bIsMonitoringEnabled = false;
        }


        private void Form1_Unload(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }


        private void CleanupCfbContext()
        {
            Log("Cleaning up context...");
            Core.CleanupCfbContext();

            Log("Unloading service and driver...");
            if (!Core.UnloadDriver())
            {
                Log("UnloadDriver() failed");
            }

            Log("Success...");
            bIsDriverLoaded = false;
            bIsMonitoringEnabled = false;
        }


        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
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
                string IrpDataStr = "";
                foreach( byte c in SelectedIrp.Body )
                {
                    IrpDataStr += $"\\x{c:X2}";
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


def Hexdump(src, length=16):    FILTER = ''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)])    lines = []    for c in range(0, len(src), length):        chars = src[c:c+length]        hex = ' '.join(['%02x' % ord(x) for x in chars])        printable = ''.join(['%s' % ((ord(x) <= 127 and FILTER[ord(x)]) or '.') for x in chars])
        lines.append('%04x  %-*s  %s\n' % (c, length * 3, hex, printable))    return ''.join(lines)


@contextmanagerdef GetDeviceHandle(DeviceName, *args, **kwargs):    Access = kwargs.get('dwDesiredAccess', win32con.GENERIC_READ | win32con.GENERIC_WRITE)    handle = kernel32.CreateFileA(DeviceName, Access, 0, None, win32con.OPEN_EXISTING, 0, None)    if handle == -1: raise IOError('Cannot get handle to %s' % DeviceName)    try: yield handle    finally: kernel32.CloseHandle(handle)


def DeviceIoctlControl(DeviceName, IoctlCode, _in='', _out='', *args, **kwargs):    dwBytesReturned = c_uint32()    InputBufferSize = kwargs.get('_inlen', len(_in))    OutputBufferSize = kwargs.get('_outlen', len(_out))    InputBuffer = create_string_buffer(InputBufferSize)    OutputBuffer = create_string_buffer(OutputBufferSize)
    InputBuffer.value = _in    OutputBuffer.value = _out    res = -1
    with GetDeviceHandle(DeviceName) as hDriver:        KdPrint('Sending inbuflen=%dB to %s with ioctl=%#x (outbuflen=%dB)' % (InputBufferSize, DeviceName, IoctlCode, OutputBufferSize))        res = kernel32.DeviceIoControl(hDriver, IoctlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, byref(dwBytesReturned), None)        KdPrint('Sent %dB to %s with IoctlCode %#x' % (InputBufferSize, DeviceName, IoctlCode ))        if res and dwBytesReturned: print(Hexdump(OutputBuffer))    return res


def Trigger():
    DeviceName = r'''{DeviceName:s}'''
    IoctlCode = 0x{SelectedIrp.Header.IoctlCode:x}
    lpIrpData = b'{IrpDataStr:s}'
    return DeviceIoctlControl(DeviceName, IoctlCode, lpIrpData)


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

        private void MenuBar_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
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

            // propagate
            monitoringToolStripMenuItem_Click(sender, e);
        }

        
        private void monitoringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (bIsMonitoringEnabled)
            {
                startMonitoringToolStripMenuItem.Enabled = false;
                stopMonitoringToolStripMenuItem.Enabled = true;
            }
            else
            {
                startMonitoringToolStripMenuItem.Enabled = true;
                stopMonitoringToolStripMenuItem.Enabled = false;
            }
        }

        private void loadDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            InitCfbContext();
            bIsDriverLoaded = true;
        }

        private void unloadIrpDumperDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CleanupCfbContext();
            bIsDriverLoaded = false;
        }

        private void startMonitoringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Log("Starting monitoring...");
            StartListening();
            Core.EnableMonitoring();
            bIsMonitoringEnabled = true;
        }

        private void stopMonitoringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Log("Stopping monitoring...");
            StopListening();
            Core.DisableMonitoring();
            bIsMonitoringEnabled = false;
        }

        private void byPathToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string DriverName = Prompt.ShowDialog("Enter the complete path to the driver object (example '\\driver\\http'):", "Manual driver selection");
            Log(String.Format("Trying to hook '{0:s}'", DriverName));

            if (!Core.HookDriver(DriverName))
            {
                Log(String.Format("Failed to hook '{0:s}'", DriverName));
                ldForm.LoadedDrivers.Add(DriverName);
                return;
            }

            Log(String.Format("Driver object '{0:s}' is now hooked.", DriverName));
        }

        private void fromListToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form fc = Application.OpenForms["LoadDriverForm"];

            if (fc == null || fc.Visible == false)
            {
                ldForm.RefreshDriverList();
                ldForm.Show();
            }
        }


    }
}
