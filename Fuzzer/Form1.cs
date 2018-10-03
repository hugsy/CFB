using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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


        public Form1()
        {
            InitializeComponent();
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

            LoadDriverBtn.Enabled = true;
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = false;
            UnloadDriverBtn.Enabled = false;

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


            Log("All good, you can start monitoring...");
            LoadDriverBtn.Enabled = false;
            StartMonitorBtn.Enabled = true;
            UnloadDriverBtn.Enabled = true;
       
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

            LoadDriverBtn.Enabled = true;
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = false;
            UnloadDriverBtn.Enabled = false;
            FuzzIrpBtn.Enabled = false;
        }


        private void StartMonitorBtn_Click(object sender, EventArgs e)
        {
            Log("Starting monitoring");           
            StartListening();
            Core.EnableMonitoring();
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = true;
        }


        private void StopMonitorBtn_Click(object sender, EventArgs e)
        {
            Log("Stopping monitoring");
            StopListening();
            Core.DisableMonitoring();
            StartMonitorBtn.Enabled = true;
            StopMonitorBtn.Enabled = false;
        }


        private void UnloadDriverBtn_Click(object sender, EventArgs e)
        {
            CleanupCfbContext();
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

        private void LoadDriverBtn_Click(object sender, EventArgs e)
        {
            InitCfbContext();
        }


        private void ShowIrpBtn_Click(object sender, EventArgs e)
        {
            Int32 selectedCellCount = IrpDataView.GetCellCount(DataGridViewElementStates.Selected);
            if (selectedCellCount == 1)
            {
                var CurrentCell = IrpDataView.SelectedCells[0];
                var Index = CurrentCell.RowIndex;
                var Irp = CfbReader.Irps[Index];

                HexViewerForm hvForm = new HexViewerForm(Index, Irp);
                hvForm.Show();
            }
        }

        private void DumpToFileBtn_Click(object sender, EventArgs e)
        {
            Int32 selectedCellCount = IrpDataView.GetCellCount(DataGridViewElementStates.Selected);
            if (selectedCellCount == 1)
            {
                var CurrentCell = IrpDataView.SelectedCells[0];
                var Irp = CfbReader.Irps[CurrentCell.RowIndex];

                SaveFileDialog saveFileDialog = new SaveFileDialog
                {
                    Filter = "Raw|*.raw",
                    Title = "Save IRP body to file"
                };
                saveFileDialog.ShowDialog();

                if (saveFileDialog.FileName != "")
                {
                    System.IO.FileStream fs = (System.IO.FileStream)saveFileDialog.OpenFile();
                    fs.Write(Irp.Body, 0, Irp.Body.Length);
                    fs.Close();
                    Log($"Saved as '{saveFileDialog.FileName:s}'");
                }
            }
        }


        private void hookUnhookDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form fc = Application.OpenForms["LoadDriverForm"];

            if (fc == null || fc.Visible == false)
            {
                ldForm.RefreshDriverList();
                ldForm.Show();
            }
        }


        private void hookUnhookDriverFromNameToolStripMenuItem_Click(object sender, EventArgs e)
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



        private void IrpDataView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {
            Int32 selectedCellCount = IrpDataView.GetCellCount(DataGridViewElementStates.Selected);
            if (selectedCellCount == 1)
            {
                ShowIrpBtn.Enabled = true;
                DumpToFileBtn.Enabled = true;
                SaveForReplayBtn.Enabled = true;
            }
            else
            {
                ShowIrpBtn.Enabled = false;
                DumpToFileBtn.Enabled = false;
                SaveForReplayBtn.Enabled = false;
            }
            return;
        }


        private void IrpDataView_Scroll(Object sender, ScrollEventArgs e)
        {
        }




        private void SaveForReplayBtn_Click(object sender, EventArgs e)
        {
            Int32 selectedCellCount = IrpDataView.GetCellCount(DataGridViewElementStates.Selected);
            if (selectedCellCount == 1)
            {
                var CurrentCell = IrpDataView.SelectedCells[0];
                var Irp = CfbReader.Irps[CurrentCell.RowIndex];

                SaveFileDialog saveFileDialog = new SaveFileDialog
                {
                    Filter = "Python|*.py",
                    Title = "Save IRP body to file"
                };
                saveFileDialog.ShowDialog();

                if (saveFileDialog.FileName != "")
                {
                    string IrpDataStr = "";
                    foreach( byte c in Irp.Body )
                    {
                        IrpDataStr += $"\\x{c:X2}";
                    }

                    string DeviceName = Irp.DeviceName.Replace("\\Device", "\\\\.");

                    string PythonReplayTemplate = 
$@"#
# Replay IOCTL 0x{Irp.Header.IoctlCode:x} script to {Irp.DeviceName:s}
# Auto-generated by CFB
# 
from ctypes import *

ntdll = windll.ntdll
kernel32 = windll.kernel32

GENERIC_READ  = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 0x03

def KdPrint(message):
    print(message)
    kernel32.OutputDebugStringA(message + '\n')
    return

def Trigger():
    lpIrpData = b'{IrpDataStr:s}'
    dwBytesReturned = c_uint32()
    hDriver = kernel32.CreateFileA(r'''{DeviceName:s}''', GENERIC_READ | GENERIC_WRITE, 0, None, OPEN_EXISTING, 0, None)
    KdPrint(r'Opened handle to device {DeviceName:s}')
    kernel32.DeviceIoControl(hDriver, 0x{Irp.Header.IoctlCode:x}, lpIrpData, len(lpIrpData), None, 0, byref(dwBytesReturned), None)
    KdPrint('Sent {Irp.Header.BufferLength:d} Bytes to IOCTL #{Irp.Header.IoctlCode:x}')
    kernel32.CloseHandle(hDriver)
    KdPrint(r'Closed handle to {DeviceName:s}')
    return 

if __name__ == '__main__':
    Trigger()
";
                    System.IO.File.WriteAllText(saveFileDialog.FileName, PythonReplayTemplate);

                    Log($"Saved as '{saveFileDialog.FileName:s}'");
                }
            }
        }

        private void FuzzIrpBtn_Click(object sender, EventArgs e)
        {

        }
    }
}
