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
            IrpDataView.DataSource = CfbReader.Messages;
            ldForm = new LoadDriverForm(this);
        }

        public void Log(string message)
        {
            string line = String.Format("[TID={0:d}] {1:s}\n", Thread.CurrentThread.ManagedThreadId, message);
            LogMutex.WaitOne();
            LogTextBox.AppendText(line);
            LogMutex.ReleaseMutex();
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
            if (CfbReader.IsThreadRunning)
                StopListening();

            CleanupCfbContext();
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            InitCfbContext();
            ShowIrpBtn.Enabled = true;
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

        }


        private void StartMonitorBtn_Click(object sender, EventArgs e)
        {
            Log("Starting monitoring");
            StartListening();
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = true;
        }


        private void StopMonitorBtn_Click(object sender, EventArgs e)
        {
            Log("Stopping monitoring");
            StopListening();
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
            }
            else
            {
                ShowIrpBtn.Enabled = false;
                DumpToFileBtn.Enabled = false;
            }
            return;
        }
    }
}
