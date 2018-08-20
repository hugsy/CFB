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
        private NamedPipeDataReader PipeReader;

        public Form1()
        {
            InitializeComponent();
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(OnProcessExit);
            PipeReader = new NamedPipeDataReader(this);
            IrpDataView.DataSource = PipeReader.Messages;
        }

        public void Log(string message)
        {
            LogTextBox.AppendText(message + "\n");
        }

        private void StartListening()
        {
            PipeReader.StartClientThread();
        }

        private void StopListening()
        {
            PipeReader.EndClientThread();
        }

        private void OnProcessExit(object sender, EventArgs e)
        {
            if (PipeReader.IsThreadRunning)
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


            Log("Creating named pipe...");
            if (!Core.CreateCfbPipe())
            {
                MessageBox.Show("CreateCfbPipe() failed");
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
                Application.Exit();
            }


            Log("All good, you can start monitoring...");
            LoadDriverBtn.Enabled = false;
            StartMonitorBtn.Enabled = true;
            UnloadDriverBtn.Enabled = true;

            // test
            Log("Hooking HEVD");

            if (!Core.HookDriver("\\driver\\HEVD"))
            {
                Log("HookDriver(HEVD) failed");
            }
            else
            {
                Log("HEVD is hooked.");
            }

        }


        private void Form1_Unload(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }

        private void CleanupCfbContext()
        {
            // test
            Log("Unhooking HEVD");

            if (!Core.UnhookDriver("\\driver\\HEVD"))
            {
                Log("UnhookDriver(HEVD) failed");
            }
            else
            {
                Log("HEVD is unhooked.");
            }


            Log("Cleaning up context...");

            Core.CleanupCfbContext();


            Log("Closing named pipe...");
            if (!Core.CloseCfbPipe())
            {
                Log("CloseCfbPipe() failed");
            }


            Log("Unloading service and driver...");
            if (!Core.UnloadDriver())
            {
                Log("UnloadDriver() failed");
            }

            Log("Success...");

            LoadDriverBtn.Enabled = true;
            StartMonitorBtn.Enabled = false;
            UnloadDriverBtn.Enabled = false;
            LoadDriverBtn.Enabled = false;
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

        private void IrpDataView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void UnloadDriverBtn_Click(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            StopListening();
            CleanupCfbContext();
            Application.Exit();
        }
    }
}
