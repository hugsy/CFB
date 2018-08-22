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



namespace Fuzzer
{
    public partial class Form1 : Form
    {
        private CfbDataReader CfbReader;
        private LoadDriverForm ldForm;

        public Form1()
        {
            InitializeComponent();
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(OnProcessExit);
            CfbReader = new CfbDataReader(this);
            IrpDataView.DataSource = CfbReader.Messages;
            ldForm = new LoadDriverForm(this);
        }

        public void Log(string message)
        {
            LogTextBox.AppendText(message + "\n");
        }

        private void StartListening()
        {
            CfbReader.StartClientThread();
        }

        private void StopListening()
        {
            CfbReader.EndClientThread();
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
            // todo: collect Data from selected IRP text
            var Data = new byte[0x100];
            for (var i = 0; i < Data.Length; i++) Data[i] = 0x42;

            HexViewerForm hvForm = new HexViewerForm(42, 1337, Data);
            hvForm.Show();
        }


        private void hookUnhookDriverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form fc = Application.OpenForms["LoadDriverForm"];

            if (fc == null || fc.Visible == false)
            {
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
                return;
            }

            Log(String.Format("Driver object '{0:s}' is now hooked.", DriverName));
        }
    }
}
