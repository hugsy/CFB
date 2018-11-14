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
        private Label label1;
        private Label label2;
        private System.ComponentModel.BackgroundWorker worker;

        public SimpleFuzzerForm(Irp irp)
        {
            InitializeComponent();
            this.Irp = irp;
            this.Text = "Fuzzing " + this.Irp.ToString();
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

            uint NbCases = 100000; // TODO: make this a global config setting

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

            if( FuzzOne(TestCaseIndex) == false )
            {
                e.Cancel = true;
                return;
            }
        }


        private bool FuzzOne(int Index)
        {
            IntPtr hDriver = Kernel32.CreateFile(
                this.Irp.DeviceName.Replace("\\Device\\", "\\\\.\\"),
                Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                0,
                IntPtr.Zero,
                Kernel32.OPEN_EXISTING,
                0,
                IntPtr.Zero
                );

            if (hDriver.ToInt32() == Kernel32.INVALID_HANDLE_VALUE )
            {
                resultLabel.Text = $"Cannot open device '{this.Irp.DeviceName}'";
                label1.Text = $" CreateFile() failed: {Kernel32.GetLastError().ToString("x8")}";
                return false;
            }

            Irp FuzzedIrp = this.Irp.Clone();
            FuzzedIrp.FuzzBody();

            IntPtr InputBuffer = Marshal.AllocHGlobal((int)FuzzedIrp.Header.InputBufferLength);
            Marshal.Copy(FuzzedIrp.Body, 0, InputBuffer, (int)FuzzedIrp.Header.InputBufferLength);

            IntPtr pdwBytesReturned = Marshal.AllocHGlobal(sizeof(int));

            IntPtr lpOutBuffer = IntPtr.Zero;
            int dwOutBufferLen = 0;

            if( FuzzedIrp.Header.OutputBufferLength > 0 )
            {
                dwOutBufferLen = ( int )FuzzedIrp.Header.OutputBufferLength;
                lpOutBuffer = Marshal.AllocHGlobal(dwOutBufferLen);
                // todo : add some checks after the devioctl for some memleaks
            }

            bool res = Kernel32.DeviceIoControl(
                hDriver,
                FuzzedIrp.Header.IoctlCode,
                InputBuffer,
                FuzzedIrp.Header.InputBufferLength,
                lpOutBuffer,
                (uint)dwOutBufferLen,
                pdwBytesReturned,
                IntPtr.Zero
                );

            Marshal.FreeHGlobal(pdwBytesReturned);
            Marshal.FreeHGlobal(InputBuffer);

            if( dwOutBufferLen > 0 )
            {
                Marshal.FreeHGlobal(lpOutBuffer);
            }

            Kernel32.CloseHandle(hDriver);

            
            if( res == false )
            {
                label1.Text = $"[{Index.ToString()}] Last request returned: {res.ToString()}";
                label1.Text+= $" - {Kernel32.GetLastError().ToString("x8")}";
            }

            return true;
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // startAsyncButton
            // 
            this.startAsyncButton.Location = new System.Drawing.Point(37, 137);
            this.startAsyncButton.Name = "startAsyncButton";
            this.startAsyncButton.Size = new System.Drawing.Size(120, 30);
            this.startAsyncButton.TabIndex = 1;
            this.startAsyncButton.Text = "Start Run";
            this.startAsyncButton.Click += new System.EventHandler(this.StartAsyncButton_Click);
            // 
            // cancelAsyncButton
            // 
            this.cancelAsyncButton.Enabled = false;
            this.cancelAsyncButton.Location = new System.Drawing.Point(445, 137);
            this.cancelAsyncButton.Name = "cancelAsyncButton";
            this.cancelAsyncButton.Size = new System.Drawing.Size(119, 30);
            this.cancelAsyncButton.TabIndex = 2;
            this.cancelAsyncButton.Text = "Cancel Run";
            this.cancelAsyncButton.Click += new System.EventHandler(this.CancelAsyncButton_Click);
            // 
            // resultLabel
            // 
            this.resultLabel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.resultLabel.Location = new System.Drawing.Point(192, 16);
            this.resultLabel.Name = "resultLabel";
            this.resultLabel.Size = new System.Drawing.Size(372, 23);
            this.resultLabel.TabIndex = 3;
            this.resultLabel.Text = "Not started";
            this.resultLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // statusLabel
            // 
            this.statusLabel.Location = new System.Drawing.Point(44, 16);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(79, 23);
            this.statusLabel.TabIndex = 3;
            this.statusLabel.Text = "Current state";
            this.statusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(37, 97);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(527, 25);
            this.progressBar1.Step = 2;
            this.progressBar1.TabIndex = 4;
            // 
            // worker
            // 
            this.worker.WorkerReportsProgress = true;
            this.worker.WorkerSupportsCancellation = true;
            // 
            // label1
            // 
            this.label1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.label1.Location = new System.Drawing.Point(192, 59);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(372, 23);
            this.label1.TabIndex = 5;
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(44, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(79, 23);
            this.label2.TabIndex = 6;
            this.label2.Text = "GetLastError()";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // SimpleFuzzerForm
            // 
            this.ClientSize = new System.Drawing.Size(599, 182);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
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