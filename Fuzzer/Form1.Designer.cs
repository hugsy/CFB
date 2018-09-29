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
            this.hookUnhookDriverToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.hookUnhookDriverFromNameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusBar = new System.Windows.Forms.ToolStripStatusLabel();
            this.LoadDriverBtn = new System.Windows.Forms.Button();
            this.UnloadDriverBtn = new System.Windows.Forms.Button();
            this.ShowIrpBtn = new System.Windows.Forms.Button();
            this.DumpToFileBtn = new System.Windows.Forms.Button();
            this.SaveForReplayBtn = new System.Windows.Forms.Button();
            this.FuzzIrpBtn = new System.Windows.Forms.Button();
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
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(681, 105);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Output";
            // 
            // LogTextBox
            // 
            this.LogTextBox.BackColor = System.Drawing.SystemColors.Control;
            this.LogTextBox.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LogTextBox.Location = new System.Drawing.Point(5, 18);
            this.LogTextBox.Multiline = true;
            this.LogTextBox.Name = "LogTextBox";
            this.LogTextBox.ReadOnly = true;
            this.LogTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.LogTextBox.Size = new System.Drawing.Size(667, 75);
            this.LogTextBox.TabIndex = 0;
            // 
            // StartMonitorBtn
            // 
            this.StartMonitorBtn.Location = new System.Drawing.Point(810, 57);
            this.StartMonitorBtn.Margin = new System.Windows.Forms.Padding(2);
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
            this.StopMonitorBtn.Location = new System.Drawing.Point(810, 97);
            this.StopMonitorBtn.Margin = new System.Windows.Forms.Padding(2);
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
            this.groupBox2.Location = new System.Drawing.Point(26, 163);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox2.Size = new System.Drawing.Size(749, 394);
            this.groupBox2.TabIndex = 5;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "IRPs";
            // 
            // IrpDataView
            // 
            this.IrpDataView.AllowUserToAddRows = false;
            this.IrpDataView.AllowUserToDeleteRows = false;
            this.IrpDataView.AllowUserToResizeRows = false;
            this.IrpDataView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.IrpDataView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.IrpDataView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.IrpDataView.Location = new System.Drawing.Point(2, 15);
            this.IrpDataView.Margin = new System.Windows.Forms.Padding(2);
            this.IrpDataView.Name = "IrpDataView";
            this.IrpDataView.ReadOnly = true;
            this.IrpDataView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.AutoSizeToAllHeaders;
            this.IrpDataView.RowTemplate.Height = 28;
            this.IrpDataView.Size = new System.Drawing.Size(745, 377);
            this.IrpDataView.TabIndex = 3;
            this.IrpDataView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.IrpDataView_CellContentClick);
            
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
            this.hookUnhookDriverToolStripMenuItem,
            this.hookUnhookDriverFromNameToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // hookUnhookDriverToolStripMenuItem
            // 
            this.hookUnhookDriverToolStripMenuItem.Name = "hookUnhookDriverToolStripMenuItem";
            this.hookUnhookDriverToolStripMenuItem.Size = new System.Drawing.Size(254, 22);
            this.hookUnhookDriverToolStripMenuItem.Text = "Hook / Unhook Driver from List";
            this.hookUnhookDriverToolStripMenuItem.Click += new System.EventHandler(this.hookUnhookDriverToolStripMenuItem_Click);
            // 
            // hookUnhookDriverFromNameToolStripMenuItem
            // 
            this.hookUnhookDriverFromNameToolStripMenuItem.Name = "hookUnhookDriverFromNameToolStripMenuItem";
            this.hookUnhookDriverFromNameToolStripMenuItem.Size = new System.Drawing.Size(254, 22);
            this.hookUnhookDriverFromNameToolStripMenuItem.Text = "Hook / Unhook Driver from Name";
            this.hookUnhookDriverFromNameToolStripMenuItem.Click += new System.EventHandler(this.hookUnhookDriverFromNameToolStripMenuItem_Click);
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(254, 22);
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
            this.LoadDriverBtn.Location = new System.Drawing.Point(711, 57);
            this.LoadDriverBtn.Margin = new System.Windows.Forms.Padding(2);
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
            this.UnloadDriverBtn.Location = new System.Drawing.Point(711, 97);
            this.UnloadDriverBtn.Margin = new System.Windows.Forms.Padding(2);
            this.UnloadDriverBtn.Name = "UnloadDriverBtn";
            this.UnloadDriverBtn.Size = new System.Drawing.Size(95, 36);
            this.UnloadDriverBtn.TabIndex = 9;
            this.UnloadDriverBtn.Text = "Unload Driver";
            this.UnloadDriverBtn.UseVisualStyleBackColor = true;
            this.UnloadDriverBtn.Click += new System.EventHandler(this.UnloadDriverBtn_Click);
            // 
            // ShowIrpBtn
            // 
            this.ShowIrpBtn.Enabled = false;
            this.ShowIrpBtn.Location = new System.Drawing.Point(792, 177);
            this.ShowIrpBtn.Margin = new System.Windows.Forms.Padding(2);
            this.ShowIrpBtn.Name = "ShowIrpBtn";
            this.ShowIrpBtn.Size = new System.Drawing.Size(111, 36);
            this.ShowIrpBtn.TabIndex = 10;
            this.ShowIrpBtn.Text = "Show IRP";
            this.ShowIrpBtn.UseVisualStyleBackColor = true;
            this.ShowIrpBtn.Click += new System.EventHandler(this.ShowIrpBtn_Click);
            // 
            // DumpToFileBtn
            // 
            this.DumpToFileBtn.Enabled = false;
            this.DumpToFileBtn.Location = new System.Drawing.Point(792, 224);
            this.DumpToFileBtn.Margin = new System.Windows.Forms.Padding(2);
            this.DumpToFileBtn.Name = "DumpToFileBtn";
            this.DumpToFileBtn.Size = new System.Drawing.Size(111, 36);
            this.DumpToFileBtn.TabIndex = 11;
            this.DumpToFileBtn.Text = "Dump to File";
            this.DumpToFileBtn.UseVisualStyleBackColor = true;
            this.DumpToFileBtn.Click += new System.EventHandler(this.DumpToFileBtn_Click);
            // 
            // SaveForReplayBtn
            // 
            this.SaveForReplayBtn.Enabled = false;
            this.SaveForReplayBtn.Location = new System.Drawing.Point(792, 274);
            this.SaveForReplayBtn.Margin = new System.Windows.Forms.Padding(2);
            this.SaveForReplayBtn.Name = "SaveForReplayBtn";
            this.SaveForReplayBtn.Size = new System.Drawing.Size(111, 36);
            this.SaveForReplayBtn.TabIndex = 12;
            this.SaveForReplayBtn.Text = "Save for Replay";
            this.SaveForReplayBtn.UseVisualStyleBackColor = true;
            this.SaveForReplayBtn.Click += new System.EventHandler(this.SaveForReplayBtn_Click);
            // 
            // FuzzIrpBtn
            // 
            this.FuzzIrpBtn.Enabled = false;
            this.FuzzIrpBtn.Location = new System.Drawing.Point(792, 325);
            this.FuzzIrpBtn.Margin = new System.Windows.Forms.Padding(2);
            this.FuzzIrpBtn.Name = "FuzzIrpBtn";
            this.FuzzIrpBtn.Size = new System.Drawing.Size(111, 36);
            this.FuzzIrpBtn.TabIndex = 13;
            this.FuzzIrpBtn.Text = "Fuzz Selected IRP";
            this.FuzzIrpBtn.UseVisualStyleBackColor = true;
            this.FuzzIrpBtn.Click += new System.EventHandler(this.FuzzIrpBtn_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(914, 603);
            this.Controls.Add(this.FuzzIrpBtn);
            this.Controls.Add(this.SaveForReplayBtn);
            this.Controls.Add(this.DumpToFileBtn);
            this.Controls.Add(this.ShowIrpBtn);
            this.Controls.Add(this.UnloadDriverBtn);
            this.Controls.Add(this.LoadDriverBtn);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.StopMonitorBtn);
            this.Controls.Add(this.StartMonitorBtn);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.MenuBar);
            this.Margin = new System.Windows.Forms.Padding(2);
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
        public System.Windows.Forms.DataGridView IrpDataView;
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
        private System.Windows.Forms.Button ShowIrpBtn;
        private System.Windows.Forms.Button DumpToFileBtn;
        private System.Windows.Forms.ToolStripMenuItem hookUnhookDriverToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem hookUnhookDriverFromNameToolStripMenuItem;
        private System.Windows.Forms.Button SaveForReplayBtn;
        private System.Windows.Forms.Button FuzzIrpBtn;
    }
}

