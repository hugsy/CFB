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

        public Form1()
        {
            InitializeComponent();
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(OnProcessExit);
            CfbReader = new CfbDataReader(this);
            IrpDataView.DataSource = CfbReader.Messages;
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
            //InitCfbContext();
            ShowIrpBtn.Enabled = true;
        }


        private void InitCfbContext()
        {
            Log("Initializing CFB...");

            LoadDriverBtn.Enabled = true;
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = false;
            UnloadDriverBtn.Enabled = false;

            // TODO : exit cleanly when one of the checks fails
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
                Application.Exit();
            }


            Log("All good, you can start monitoring...");
            LoadDriverBtn.Enabled = false;
            StartMonitorBtn.Enabled = true;
            UnloadDriverBtn.Enabled = true;

            // test
            Log("Hooking hevd");

            if (!Core.HookDriver("\\driver\\hevd"))
            {
                Log("HookDriver(hevd) failed");
            }
            else
            {
                Log("hevd is hooked.");
            }

        }


        private void Form1_Unload(object sender, EventArgs e)
        {
            CleanupCfbContext();
        }

        private void CleanupCfbContext()
        {
            // test
            Log("Unhooking hevd");

            if (!Core.UnhookDriver("\\driver\\hevd"))
            {
                Log("UnhookDriver(hevd) failed");
            }
            else
            {
                Log("hevd is unhooked.");
            }


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
            StopListening();
            CleanupCfbContext();
            Application.Exit();
        }

        private void LoadDriverBtn_Click(object sender, EventArgs e)
        {

        }

        private void ShowIrpBtn_Click(object sender, EventArgs e)
        {
            var Data = new byte[0x100];
            for (var i = 0; i < Data.Length; i++) Data[i] = 0x42;

            HexViewerForm f = new HexViewerForm(1337, Data);
            f.Show();
        }
    }
}
