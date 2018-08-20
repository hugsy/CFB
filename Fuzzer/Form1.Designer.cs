namespace Fuzzer
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.LogTextBox = new System.Windows.Forms.TextBox();
            this.StartMonitorBtn = new System.Windows.Forms.Button();
            this.StopMonitorBtn = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.IrpDataView = new System.Windows.Forms.DataGridView();
            this.IrpTimeStamp = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpProcessId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpIoctlCode = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpRequestLevel = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Action = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.MenuBar = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusBar = new System.Windows.Forms.ToolStripStatusLabel();
            this.LoadDriverBtn = new System.Windows.Forms.Button();
            this.UnloadDriverBtn = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.IrpDataView)).BeginInit();
            this.MenuBar.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.LogTextBox);
            this.groupBox1.Location = new System.Drawing.Point(26, 42);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.groupBox1.Size = new System.Drawing.Size(717, 198);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Output";
            // 
            // LogTextBox
            // 
            this.LogTextBox.Location = new System.Drawing.Point(18, 18);
            this.LogTextBox.Multiline = true;
            this.LogTextBox.Name = "LogTextBox";
            this.LogTextBox.ReadOnly = true;
            this.LogTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.LogTextBox.Size = new System.Drawing.Size(681, 162);
            this.LogTextBox.TabIndex = 0;
            // 
            // StartMonitorBtn
            // 
            this.StartMonitorBtn.Location = new System.Drawing.Point(779, 99);
            this.StartMonitorBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.StartMonitorBtn.Name = "StartMonitorBtn";
            this.StartMonitorBtn.Size = new System.Drawing.Size(95, 36);
            this.StartMonitorBtn.TabIndex = 1;
            this.StartMonitorBtn.Text = "Start Monitoring";
            this.StartMonitorBtn.UseVisualStyleBackColor = true;
            this.StartMonitorBtn.Click += new System.EventHandler(this.StartMonitorBtn_Click);
            // 
            // StopMonitorBtn
            // 
            this.StopMonitorBtn.Enabled = false;
            this.StopMonitorBtn.Location = new System.Drawing.Point(779, 143);
            this.StopMonitorBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.StopMonitorBtn.Name = "StopMonitorBtn";
            this.StopMonitorBtn.Size = new System.Drawing.Size(95, 36);
            this.StopMonitorBtn.TabIndex = 2;
            this.StopMonitorBtn.Text = "Stop Monitoring";
            this.StopMonitorBtn.UseVisualStyleBackColor = true;
            this.StopMonitorBtn.Click += new System.EventHandler(this.StopMonitorBtn_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.IrpDataView);
            this.groupBox2.Location = new System.Drawing.Point(26, 254);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.groupBox2.Size = new System.Drawing.Size(847, 311);
            this.groupBox2.TabIndex = 5;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "IRPs";
            // 
            // IrpDataView
            // 
            this.IrpDataView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.AllCells;
            this.IrpDataView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.IrpDataView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.IrpDataView.Location = new System.Drawing.Point(2, 15);
            this.IrpDataView.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.IrpDataView.Name = "IrpDataView";
            this.IrpDataView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.AutoSizeToAllHeaders;
            this.IrpDataView.RowTemplate.Height = 28;
            this.IrpDataView.Size = new System.Drawing.Size(843, 294);
            this.IrpDataView.TabIndex = 3;
            // 
            // IrpTimeStamp
            // 
            this.IrpTimeStamp.Name = "IrpTimeStamp";
            // 
            // IrpProcessId
            // 
            this.IrpProcessId.Name = "IrpProcessId";
            // 
            // IrpIoctlCode
            // 
            this.IrpIoctlCode.Name = "IrpIoctlCode";
            // 
            // IrpRequestLevel
            // 
            this.IrpRequestLevel.Name = "IrpRequestLevel";
            // 
            // Action
            // 
            this.Action.Name = "Action";
            // 
            // MenuBar
            // 
            this.MenuBar.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.MenuBar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.MenuBar.Location = new System.Drawing.Point(0, 0);
            this.MenuBar.Name = "MenuBar";
            this.MenuBar.Size = new System.Drawing.Size(914, 24);
            this.MenuBar.TabIndex = 6;
            this.MenuBar.Text = "CFB";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(97, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            this.quitToolStripMenuItem.Click += new System.EventHandler(this.quitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.aboutToolStripMenuItem.Text = "About";
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatusBar});
            this.statusStrip1.Location = new System.Drawing.Point(0, 581);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(914, 22);
            this.statusStrip1.TabIndex = 7;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // StatusBar
            // 
            this.StatusBar.Name = "StatusBar";
            this.StatusBar.Size = new System.Drawing.Size(28, 17);
            this.StatusBar.Text = "CFB";
            // 
            // LoadDriverBtn
            // 
            this.LoadDriverBtn.Location = new System.Drawing.Point(779, 55);
            this.LoadDriverBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.LoadDriverBtn.Name = "LoadDriverBtn";
            this.LoadDriverBtn.Size = new System.Drawing.Size(95, 36);
            this.LoadDriverBtn.TabIndex = 8;
            this.LoadDriverBtn.Text = "Load driver";
            this.LoadDriverBtn.UseVisualStyleBackColor = true;
            this.LoadDriverBtn.Click += new System.EventHandler(this.LoadDriverBtn_Click);
            // 
            // UnloadDriverBtn
            // 
            this.UnloadDriverBtn.Enabled = false;
            this.UnloadDriverBtn.Location = new System.Drawing.Point(779, 188);
            this.UnloadDriverBtn.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.UnloadDriverBtn.Name = "UnloadDriverBtn";
            this.UnloadDriverBtn.Size = new System.Drawing.Size(95, 36);
            this.UnloadDriverBtn.TabIndex = 9;
            this.UnloadDriverBtn.Text = "UnloadDriver";
            this.UnloadDriverBtn.UseVisualStyleBackColor = true;
            this.UnloadDriverBtn.Click += new System.EventHandler(this.UnloadDriverBtn_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(914, 603);
            this.Controls.Add(this.UnloadDriverBtn);
            this.Controls.Add(this.LoadDriverBtn);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.StopMonitorBtn);
            this.Controls.Add(this.StartMonitorBtn);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.MenuBar);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "Form1";
            this.Text = "Fuzzer Engine for CFB";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.IrpDataView)).EndInit();
            this.MenuBar.ResumeLayout(false);
            this.MenuBar.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button StartMonitorBtn;
        private System.Windows.Forms.Button StopMonitorBtn;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox LogTextBox;
        private System.Windows.Forms.DataGridView IrpDataView;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpTimeStamp;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpProcessId;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpIoctlCode;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpRequestLevel;
        private System.Windows.Forms.DataGridViewTextBoxColumn Action;
        private System.Windows.Forms.MenuStrip MenuBar;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel StatusBar;
        private System.Windows.Forms.Button LoadDriverBtn;
        private System.Windows.Forms.Button UnloadDriverBtn;
    }
}

