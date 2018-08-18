using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

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

        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Starting monitoring");
            StartListening();
            button1.Enabled = false;
            button2.Enabled = true;          
        }

        private void button2_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Stopping monitoring");
            StopListening();
            button1.Enabled = true;
            button2.Enabled = false;
        }

        private void IrpDataView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

    }
}
