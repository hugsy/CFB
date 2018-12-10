using System;
using System.ComponentModel;
using System.Windows.Forms;

namespace Fuzzer
{
    public class SimpleFuzzerForm : Form
    {
        private readonly Irp Irp;

        private Button startAsyncButton;
        private Button cancelAsyncButton;
        private ProgressBar progressBar1;
        private Label statusLabel;
        private Label resultLabel;
        private Label label1;
        private Label label2;
        private Label label3;
        private Label label4;
        private Label label5;
        private Label label6;
        private TextBox FuzzByteStartIndexTextbox;
        private TextBox FuzzByteEndIndexTextbox;
        private TextBox MaxTestCaseTextbox;
        private BackgroundWorker worker;
        private ComboBox StrategyComboBox;
        private StatusStrip SimpleFuzzStatusBar;
        private ToolStripStatusLabel SimpleFuzzStatusBarItem;
        private Label label7;
        private TextBox DeviceObjectPathTextBox;
        private Label label8;
        private TextBox IoctlCodeTextBox;
        private CheckBox RandomizeInputLengthCheckbox;
        private GroupBox RandomFuzzGroupBox;
        private CheckBox RandomizeOutputLengthCheckbox;

        private FuzzingStrategy[] Strategies;
        private FuzzingSession Session;

        
        public SimpleFuzzerForm(Irp irp)
        {
            InitializeComponent();
            InitializeFuzzingObjects();
            this.Irp = irp;
            this.Text = $"Fuzzing {Irp.ToString()}";
            this.DeviceObjectPathTextBox.Text = this.Irp.DeviceName.Replace("\\Device\\", "\\\\.\\");
            this.IoctlCodeTextBox.Text = $"0x{Irp.Header.IoctlCode:x}";
            this.IoctlCodeTextBox.Enabled = false;
            InitializeWorker();
        }

        private void InitializeWorker()
        {
            worker.DoWork                += new DoWorkEventHandler(BackgroundWorkRoutine);
            worker.RunWorkerCompleted    += new RunWorkerCompletedEventHandler(OnComplete);
            worker.ProgressChanged       += new ProgressChangedEventHandler(OnProgressChange);
        }


        private void InitializeFuzzingObjects()
        {
            Session = new FuzzingSession();

            Strategies = new FuzzingStrategy[]
            {
                new RandomFuzzingStrategy(),
                new RandomAsciiFuzzingStrategy(),
                new RandomUnicodeFuzzingStrategy(),
                new BitflipFuzzingStrategy(),
                new BigintOverwriteFuzzingStrategy(),

            };

            foreach (FuzzingStrategy s in Strategies)
            {
                StrategyComboBox.Items.Add(s);
            }
        }


        private void StartAsyncButton_Click(Object sender, EventArgs e)
        {
            resultLabel.Text = String.Empty;
            this.startAsyncButton.Enabled = false;
            this.cancelAsyncButton.Enabled = true;
            worker.RunWorkerAsync();
        }

        private void CancelAsyncButton_Click(Object sender, EventArgs e)
        {
            this.worker.CancelAsync();
            resultLabel.Text = "Cancelling...";
            cancelAsyncButton.Enabled = false;
        }


        private void BackgroundWorkRoutine(object sender, DoWorkEventArgs evt)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            bool res;

            resultLabel.Text = $"Defining strategy...";
            FuzzingStrategy SelectedStrategy = (FuzzingStrategy)StrategyComboBox.SelectedItem;

            if (SelectedStrategy == null)
            {
                MessageBox.Show($"Strategy '{SelectedStrategy}' does not exist", "UnimplementedStrategy");
                return;
            }


            resultLabel.Text = $"Validating session parameters...";

            res = System.Int32.TryParse(FuzzByteStartIndexTextbox.Text, out int StartByteIndex);
            if(!res || StartByteIndex < 0 || StartByteIndex > this.Irp.Body.Length-1)
                StartByteIndex = 0;

            res = System.Int32.TryParse(FuzzByteEndIndexTextbox.Text, out int EndByteIndex);
            if (!res || EndByteIndex < 0 || EndByteIndex > this.Irp.Body.Length-1)
                EndByteIndex = this.Irp.Body.Length-1;

            if (StartByteIndex > EndByteIndex)
            {
                StartByteIndex = 0;
                EndByteIndex = this.Irp.Body.Length - 1;
            }

            string DeviceName = this.DeviceObjectPathTextBox.Text;

            resultLabel.Text = $"Initiating fuzzing with strategy '{SelectedStrategy}'...";
            Session.Start(DeviceName, SelectedStrategy, Irp, worker, evt, StartByteIndex, EndByteIndex);
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
        }
        
        
        private void StrategyComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            FuzzingStrategy SelectedStrategy = (FuzzingStrategy)StrategyComboBox.SelectedItem;
            SimpleFuzzStatusBarItem.Text = SelectedStrategy.Description;

            if ( SelectedStrategy is RandomFuzzingStrategy )
            {
                RandomFuzzGroupBox.Enabled = true;
                FuzzByteStartIndexTextbox.Enabled = true;
                FuzzByteEndIndexTextbox.Enabled = true;
                MaxTestCaseTextbox.Enabled = true;
                RandomizeInputLengthCheckbox.Enabled = true;
                RandomizeOutputLengthCheckbox.Enabled = true;
            }
            else
            {
                RandomFuzzGroupBox.Enabled = true;
                FuzzByteStartIndexTextbox.Enabled = false;
                FuzzByteEndIndexTextbox.Enabled = false;
                MaxTestCaseTextbox.Enabled = false;
                RandomizeInputLengthCheckbox.Enabled = false;
                RandomizeOutputLengthCheckbox.Enabled = false;
            }
        }


        #region Windows Form Designer generated code
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SimpleFuzzerForm));
            this.startAsyncButton = new System.Windows.Forms.Button();
            this.cancelAsyncButton = new System.Windows.Forms.Button();
            this.resultLabel = new System.Windows.Forms.Label();
            this.statusLabel = new System.Windows.Forms.Label();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.worker = new System.ComponentModel.BackgroundWorker();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.FuzzByteStartIndexTextbox = new System.Windows.Forms.TextBox();
            this.FuzzByteEndIndexTextbox = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.MaxTestCaseTextbox = new System.Windows.Forms.TextBox();
            this.StrategyComboBox = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.SimpleFuzzStatusBar = new System.Windows.Forms.StatusStrip();
            this.SimpleFuzzStatusBarItem = new System.Windows.Forms.ToolStripStatusLabel();
            this.label7 = new System.Windows.Forms.Label();
            this.DeviceObjectPathTextBox = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.IoctlCodeTextBox = new System.Windows.Forms.TextBox();
            this.RandomizeInputLengthCheckbox = new System.Windows.Forms.CheckBox();
            this.RandomFuzzGroupBox = new System.Windows.Forms.GroupBox();
            this.RandomizeOutputLengthCheckbox = new System.Windows.Forms.CheckBox();
            this.SimpleFuzzStatusBar.SuspendLayout();
            this.RandomFuzzGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // startAsyncButton
            // 
            this.startAsyncButton.Location = new System.Drawing.Point(37, 343);
            this.startAsyncButton.Name = "startAsyncButton";
            this.startAsyncButton.Size = new System.Drawing.Size(120, 30);
            this.startAsyncButton.TabIndex = 1;
            this.startAsyncButton.Text = "Start Run";
            this.startAsyncButton.Click += new System.EventHandler(this.StartAsyncButton_Click);
            // 
            // cancelAsyncButton
            // 
            this.cancelAsyncButton.Enabled = false;
            this.cancelAsyncButton.Location = new System.Drawing.Point(445, 343);
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
            this.progressBar1.Location = new System.Drawing.Point(37, 320);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(527, 17);
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
            this.label1.Location = new System.Drawing.Point(192, 51);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(372, 23);
            this.label1.TabIndex = 5;
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(44, 51);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(113, 23);
            this.label2.TabIndex = 6;
            this.label2.Text = "GetLastError()";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(7, 25);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(113, 23);
            this.label3.TabIndex = 7;
            this.label3.Text = "From Index (opt.)";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(7, 54);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(113, 23);
            this.label4.TabIndex = 8;
            this.label4.Text = "To Index (opt.)";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // FuzzByteStartIndexTextbox
            // 
            this.FuzzByteStartIndexTextbox.Location = new System.Drawing.Point(155, 27);
            this.FuzzByteStartIndexTextbox.Name = "FuzzByteStartIndexTextbox";
            this.FuzzByteStartIndexTextbox.Size = new System.Drawing.Size(100, 26);
            this.FuzzByteStartIndexTextbox.TabIndex = 9;
            // 
            // FuzzByteEndIndexTextbox
            // 
            this.FuzzByteEndIndexTextbox.Location = new System.Drawing.Point(155, 56);
            this.FuzzByteEndIndexTextbox.Name = "FuzzByteEndIndexTextbox";
            this.FuzzByteEndIndexTextbox.Size = new System.Drawing.Size(100, 26);
            this.FuzzByteEndIndexTextbox.TabIndex = 10;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(7, 82);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(113, 23);
            this.label5.TabIndex = 11;
            this.label5.Text = "Maximum Test Cases";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // MaxTestCaseTextbox
            // 
            this.MaxTestCaseTextbox.Location = new System.Drawing.Point(155, 84);
            this.MaxTestCaseTextbox.Name = "MaxTestCaseTextbox";
            this.MaxTestCaseTextbox.Size = new System.Drawing.Size(100, 26);
            this.MaxTestCaseTextbox.TabIndex = 12;
            this.MaxTestCaseTextbox.Text = "-1";
            // 
            // StrategyComboBox
            // 
            this.StrategyComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.StrategyComboBox.FormattingEnabled = true;
            this.StrategyComboBox.Location = new System.Drawing.Point(423, 128);
            this.StrategyComboBox.Name = "StrategyComboBox";
            this.StrategyComboBox.Size = new System.Drawing.Size(137, 28);
            this.StrategyComboBox.TabIndex = 13;
            this.StrategyComboBox.TabStop = false;
            this.StrategyComboBox.SelectedIndexChanged += new System.EventHandler(this.StrategyComboBox_SelectedIndexChanged);
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(344, 128);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(73, 23);
            this.label6.TabIndex = 14;
            this.label6.Text = "Strategy";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // SimpleFuzzStatusBar
            // 
            this.SimpleFuzzStatusBar.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.SimpleFuzzStatusBar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.SimpleFuzzStatusBarItem});
            this.SimpleFuzzStatusBar.Location = new System.Drawing.Point(0, 377);
            this.SimpleFuzzStatusBar.Name = "SimpleFuzzStatusBar";
            this.SimpleFuzzStatusBar.Size = new System.Drawing.Size(607, 30);
            this.SimpleFuzzStatusBar.TabIndex = 15;
            this.SimpleFuzzStatusBar.Text = "statusStrip1";
            // 
            // SimpleFuzzStatusBarItem
            // 
            this.SimpleFuzzStatusBarItem.Name = "SimpleFuzzStatusBarItem";
            this.SimpleFuzzStatusBarItem.Size = new System.Drawing.Size(60, 25);
            this.SimpleFuzzStatusBarItem.Text = "Status";
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(44, 98);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(105, 23);
            this.label7.TabIndex = 16;
            this.label7.Text = "Device Object (opt.)";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // DeviceObjectPathTextBox
            // 
            this.DeviceObjectPathTextBox.Location = new System.Drawing.Point(177, 100);
            this.DeviceObjectPathTextBox.Name = "DeviceObjectPathTextBox";
            this.DeviceObjectPathTextBox.Size = new System.Drawing.Size(140, 26);
            this.DeviceObjectPathTextBox.TabIndex = 17;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(344, 98);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(105, 23);
            this.label8.TabIndex = 18;
            this.label8.Text = "IoctlCode";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // IoctlCodeTextBox
            // 
            this.IoctlCodeTextBox.Location = new System.Drawing.Point(423, 100);
            this.IoctlCodeTextBox.Name = "IoctlCodeTextBox";
            this.IoctlCodeTextBox.Size = new System.Drawing.Size(137, 26);
            this.IoctlCodeTextBox.TabIndex = 19;
            // 
            // RandomizeInputLengthCheckbox
            // 
            this.RandomizeInputLengthCheckbox.AutoSize = true;
            this.RandomizeInputLengthCheckbox.Location = new System.Drawing.Point(310, 29);
            this.RandomizeInputLengthCheckbox.Name = "RandomizeInputLengthCheckbox";
            this.RandomizeInputLengthCheckbox.Size = new System.Drawing.Size(211, 24);
            this.RandomizeInputLengthCheckbox.TabIndex = 20;
            this.RandomizeInputLengthCheckbox.Text = "Randomize Input Length";
            this.RandomizeInputLengthCheckbox.UseVisualStyleBackColor = true;
            // 
            // RandomFuzzGroupBox
            // 
            this.RandomFuzzGroupBox.Controls.Add(this.RandomizeOutputLengthCheckbox);
            this.RandomFuzzGroupBox.Controls.Add(this.RandomizeInputLengthCheckbox);
            this.RandomFuzzGroupBox.Controls.Add(this.label3);
            this.RandomFuzzGroupBox.Controls.Add(this.label4);
            this.RandomFuzzGroupBox.Controls.Add(this.FuzzByteStartIndexTextbox);
            this.RandomFuzzGroupBox.Controls.Add(this.FuzzByteEndIndexTextbox);
            this.RandomFuzzGroupBox.Controls.Add(this.label5);
            this.RandomFuzzGroupBox.Controls.Add(this.MaxTestCaseTextbox);
            this.RandomFuzzGroupBox.Location = new System.Drawing.Point(37, 188);
            this.RandomFuzzGroupBox.Name = "RandomFuzzGroupBox";
            this.RandomFuzzGroupBox.Size = new System.Drawing.Size(527, 113);
            this.RandomFuzzGroupBox.TabIndex = 21;
            this.RandomFuzzGroupBox.TabStop = false;
            this.RandomFuzzGroupBox.Text = "Random Fuzzing Options";
            // 
            // RandomizeOutputLengthCheckbox
            // 
            this.RandomizeOutputLengthCheckbox.AutoSize = true;
            this.RandomizeOutputLengthCheckbox.Location = new System.Drawing.Point(310, 58);
            this.RandomizeOutputLengthCheckbox.Name = "RandomizeOutputLengthCheckbox";
            this.RandomizeOutputLengthCheckbox.Size = new System.Drawing.Size(223, 24);
            this.RandomizeOutputLengthCheckbox.TabIndex = 21;
            this.RandomizeOutputLengthCheckbox.Text = "Randomize Output Length";
            this.RandomizeOutputLengthCheckbox.UseVisualStyleBackColor = true;
            // 
            // SimpleFuzzerForm
            // 
            this.ClientSize = new System.Drawing.Size(607, 407);
            this.Controls.Add(this.RandomFuzzGroupBox);
            this.Controls.Add(this.IoctlCodeTextBox);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.DeviceObjectPathTextBox);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.SimpleFuzzStatusBar);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.StrategyComboBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.resultLabel);
            this.Controls.Add(this.cancelAsyncButton);
            this.Controls.Add(this.startAsyncButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "SimpleFuzzerForm";
            this.Text = "Simple IRP Fuzzer";
            this.SimpleFuzzStatusBar.ResumeLayout(false);
            this.SimpleFuzzStatusBar.PerformLayout();
            this.RandomFuzzGroupBox.ResumeLayout(false);
            this.RandomFuzzGroupBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion


    }
}