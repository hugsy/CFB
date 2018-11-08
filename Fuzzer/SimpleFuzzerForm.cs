using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Fuzzer
{
    public class SimpleFuzzerForm : System.Windows.Forms.Form
    {
        private Irp Irp;

        private System.Windows.Forms.Button startAsyncButton;
        private System.Windows.Forms.Button cancelAsyncButton;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.Label statusLabel;
        private System.Windows.Forms.Label resultLabel;
        private System.ComponentModel.BackgroundWorker worker;

        public SimpleFuzzerForm(Irp irp)
        {
            InitializeComponent();
            this.Irp = irp;
            this.Text = "Simple IRP Fuzzer: " + this.Irp.ToString();
            InitializeWorker();
        }

        private void InitializeWorker()
        {
            worker.DoWork += new DoWorkEventHandler(BackgroundWorkRoutine);
            worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(OnComplete);
            worker.ProgressChanged += new ProgressChangedEventHandler(OnProgressChange);
        }

        private void StartAsyncButton_Click(System.Object sender, System.EventArgs e)
        {
            resultLabel.Text = String.Empty;
            this.startAsyncButton.Enabled = false;
            this.cancelAsyncButton.Enabled = true;
            worker.RunWorkerAsync();
        }

        private void CancelAsyncButton_Click(System.Object sender, System.EventArgs e)
        {
            this.worker.CancelAsync();
            resultLabel.Text = "Cancelling...";
            cancelAsyncButton.Enabled = false;
        }


        private void BackgroundWorkRoutine(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;

            // Generate all test cases
            // TODO

            // Run the tests
            RunTestCases(worker, e);
        }


        private void OnComplete(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                MessageBox.Show(e.Error.Message);
            }
            else if (e.Cancelled)
            {
                resultLabel.Text = "Cancelled";
            }
            else
            {
                resultLabel.Text = "Done";
                progressBar1.Value = 0;
            }

            startAsyncButton.Enabled = true;
            cancelAsyncButton.Enabled = false;
        }

        private void OnProgressChange(object sender, ProgressChangedEventArgs e)
        {
            this.progressBar1.Value = e.ProgressPercentage;
            resultLabel.Text = this.progressBar1.Value.ToString() + "% done";
        }

        private void RunTestCases(BackgroundWorker worker, DoWorkEventArgs e)
        {
            if (worker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }

            int NbCases = 100;

            for(var i = 0; i < NbCases; i++)
            {
                RunTestCase(i, worker, e);

                if (worker.CancellationPending)
                {
                    break;
                }

                int percentComplete = (int)((float)i / (float)NbCases * 100);
                worker.ReportProgress(percentComplete);
            }

            return;
        }

        private void RunTestCase(int TestCaseIndex, BackgroundWorker worker, DoWorkEventArgs e)
        {
            if (worker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }

            System.Threading.Thread.Sleep(500);
        }


        private void FuzzOne()
        {
            IntPtr hDriver = Kernel32.CreateFile(
                this.Irp.DeviceName,
                Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                0,
                IntPtr.Zero,
                Kernel32.OPEN_EXISTING,
                0,
                IntPtr.Zero
                );

            if (hDriver == IntPtr.Zero)
            {
                resultLabel.Text = "failed to open";
                return;
            }

            Irp FuzzedIrp = this.Irp.Clone();
            FuzzedIrp.FuzzBody();

            IntPtr pdwBytesReturned = Marshal.AllocHGlobal(sizeof(int));

            Kernel32.DeviceIoControl(
                hDriver,
                FuzzedIrp.Header.IoctlCode,
                FuzzedIrp.Body,
                FuzzedIrp.Header.BufferLength,
                IntPtr.Zero,
                0,
                pdwBytesReturned,
                IntPtr.Zero
                );

            Marshal.FreeHGlobal(pdwBytesReturned);

            Kernel32.CloseHandle(hDriver);
        }



        #region Windows Form Designer generated code
        private void InitializeComponent()
        {
            this.startAsyncButton = new System.Windows.Forms.Button();
            this.cancelAsyncButton = new System.Windows.Forms.Button();
            this.resultLabel = new System.Windows.Forms.Label();
            this.statusLabel = new System.Windows.Forms.Label();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.worker = new System.ComponentModel.BackgroundWorker();
            this.SuspendLayout();
            // 
            // startAsyncButton
            // 
            this.startAsyncButton.Location = new System.Drawing.Point(18, 87);
            this.startAsyncButton.Name = "startAsyncButton";
            this.startAsyncButton.Size = new System.Drawing.Size(120, 30);
            this.startAsyncButton.TabIndex = 1;
            this.startAsyncButton.Text = "Start Run";
            this.startAsyncButton.Click += new System.EventHandler(this.StartAsyncButton_Click);
            // 
            // cancelAsyncButton
            // 
            this.cancelAsyncButton.Enabled = false;
            this.cancelAsyncButton.Location = new System.Drawing.Point(317, 87);
            this.cancelAsyncButton.Name = "cancelAsyncButton";
            this.cancelAsyncButton.Size = new System.Drawing.Size(119, 30);
            this.cancelAsyncButton.TabIndex = 2;
            this.cancelAsyncButton.Text = "Cancel Run";
            this.cancelAsyncButton.Click += new System.EventHandler(this.CancelAsyncButton_Click);
            // 
            // resultLabel
            // 
            this.resultLabel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.resultLabel.Location = new System.Drawing.Point(181, 16);
            this.resultLabel.Name = "resultLabel";
            this.resultLabel.Size = new System.Drawing.Size(255, 23);
            this.resultLabel.TabIndex = 3;
            this.resultLabel.Text = "Not started";
            this.resultLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // statusLabel
            // 
            this.statusLabel.Location = new System.Drawing.Point(16, 16);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(79, 23);
            this.statusLabel.TabIndex = 3;
            this.statusLabel.Text = "Current state";
            this.statusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(18, 48);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(420, 10);
            this.progressBar1.Step = 2;
            this.progressBar1.TabIndex = 4;
            // 
            // worker
            // 
            this.worker.WorkerReportsProgress = true;
            this.worker.WorkerSupportsCancellation = true;
            // 
            // SimpleFuzzerForm
            // 
            this.ClientSize = new System.Drawing.Size(448, 141);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.resultLabel);
            this.Controls.Add(this.cancelAsyncButton);
            this.Controls.Add(this.startAsyncButton);
            this.Name = "SimpleFuzzerForm";
            this.Text = "Simple IRP Fuzzer";
            this.ResumeLayout(false);

        }
        #endregion
    }
}