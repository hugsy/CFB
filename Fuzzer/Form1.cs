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
            PipeReader = new NamedPipeDataReader();
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
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            Log("Initializing CFB...");

            LoadDriverBtn.Enabled = true;
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = false;
            UnloadDriverBtn.Enabled = false;

            //
            //
            //
            Log("Checking privileges...");
            if (!Core.HasPrivilege("SeDebugPrivilege"))
            {
                Log("SeDebugPrivilege missing, trying to add...");
            }
            else
            {
                Log("SeDebugPrivilege missing, trying to add...");
            }


        }


        private void StartMonitorBtn_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Starting monitoring");
            StartListening();
            StartMonitorBtn.Enabled = false;
            StopMonitorBtn.Enabled = true;          
        }

        private void StopMonitorBtn_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Stopping monitoring");
            StopListening();
            StartMonitorBtn.Enabled = true;
            StopMonitorBtn.Enabled = false;
        }

        private void IrpDataView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }
    }
}
