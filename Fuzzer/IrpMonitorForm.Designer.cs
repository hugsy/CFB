using System.Windows.Forms;

namespace Fuzzer
{
    partial class IrpMonitorForm
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
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.IrpDataView = new System.Windows.Forms.DataGridView();
            this.IrpTimeStamp = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpProcessId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpIoctlCode = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpRequestLevel = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Action = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.MenuBar = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.controlToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.driverToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadDriverToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.unloadIrpDumperDriverToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.monitoringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.startMonitoringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stopMonitoringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.hookUnhookDriversToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.byPathToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fromListToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusBar = new System.Windows.Forms.ToolStripStatusLabel();
            this.ShowIrpBtn = new System.Windows.Forms.Button();
            this.DumpToFileBtn = new System.Windows.Forms.Button();
            this.SaveForReplayBtn = new System.Windows.Forms.Button();
            this.FuzzIrpBtn = new System.Windows.Forms.Button();
            this.CleanIrpDataGridButton = new System.Windows.Forms.Button();
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
            this.groupBox1.Location = new System.Drawing.Point(0, 497);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(914, 82);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Log";
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
            this.LogTextBox.Size = new System.Drawing.Size(904, 59);
            this.LogTextBox.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.IrpDataView);
            this.groupBox2.Location = new System.Drawing.Point(5, 52);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox2.Size = new System.Drawing.Size(904, 441);
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
            this.IrpDataView.MultiSelect = false;
            this.IrpDataView.Name = "IrpDataView";
            this.IrpDataView.ReadOnly = true;
            this.IrpDataView.RowHeadersVisible = false;
            this.IrpDataView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.DisableResizing;
            this.IrpDataView.RowTemplate.Height = 28;
            this.IrpDataView.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.IrpDataView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.IrpDataView.ShowCellErrors = false;
            this.IrpDataView.ShowCellToolTips = false;
            this.IrpDataView.ShowEditingIcon = false;
            this.IrpDataView.ShowRowErrors = false;
            this.IrpDataView.Size = new System.Drawing.Size(900, 424);
            this.IrpDataView.TabIndex = 3;
            this.IrpDataView.SelectionChanged += new System.EventHandler(this.IrpDataView_SelectionChanged);
            this.IrpDataView.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.IrpDataView_MouseDoubleClick);
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
            this.controlToolStripMenuItem,
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
            this.settingsToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.settingsToolStripMenuItem.Text = "Global Settings";
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            this.quitToolStripMenuItem.Click += new System.EventHandler(this.QuitToolStripMenuItem_Click);
            // 
            // controlToolStripMenuItem
            // 
            this.controlToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.driverToolStripMenuItem,
            this.monitoringToolStripMenuItem,
            this.hookUnhookDriversToolStripMenuItem});
            this.controlToolStripMenuItem.Name = "controlToolStripMenuItem";
            this.controlToolStripMenuItem.Size = new System.Drawing.Size(59, 20);
            this.controlToolStripMenuItem.Text = "Control";
            // 
            // driverToolStripMenuItem
            // 
            this.driverToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadDriverToolStripMenuItem,
            this.unloadIrpDumperDriverToolStripMenuItem});
            this.driverToolStripMenuItem.Name = "driverToolStripMenuItem";
            this.driverToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.driverToolStripMenuItem.Text = "Driver";
            // 
            // loadDriverToolStripMenuItem
            // 
            this.loadDriverToolStripMenuItem.Name = "loadDriverToolStripMenuItem";
            this.loadDriverToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.L)));
            this.loadDriverToolStripMenuItem.Size = new System.Drawing.Size(247, 22);
            this.loadDriverToolStripMenuItem.Text = "Load IrpDumper driver";
            this.loadDriverToolStripMenuItem.Click += new System.EventHandler(this.LoadDriverToolStripMenuItem_Click);
            // 
            // unloadIrpDumperDriverToolStripMenuItem
            // 
            this.unloadIrpDumperDriverToolStripMenuItem.Name = "unloadIrpDumperDriverToolStripMenuItem";
            this.unloadIrpDumperDriverToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.U)));
            this.unloadIrpDumperDriverToolStripMenuItem.Size = new System.Drawing.Size(247, 22);
            this.unloadIrpDumperDriverToolStripMenuItem.Text = "Unload IrpDumper driver";
            this.unloadIrpDumperDriverToolStripMenuItem.Click += new System.EventHandler(this.UnloadIrpDumperDriverToolStripMenuItem_Click);
            // 
            // monitoringToolStripMenuItem
            // 
            this.monitoringToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.startMonitoringToolStripMenuItem,
            this.stopMonitoringToolStripMenuItem});
            this.monitoringToolStripMenuItem.Name = "monitoringToolStripMenuItem";
            this.monitoringToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.monitoringToolStripMenuItem.Text = "Monitoring";
            // 
            // startMonitoringToolStripMenuItem
            // 
            this.startMonitoringToolStripMenuItem.Name = "startMonitoringToolStripMenuItem";
            this.startMonitoringToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.M)));
            this.startMonitoringToolStripMenuItem.Size = new System.Drawing.Size(206, 22);
            this.startMonitoringToolStripMenuItem.Text = "Start monitoring";
            this.startMonitoringToolStripMenuItem.Click += new System.EventHandler(this.StartMonitoringToolStripMenuItem_Click);
            // 
            // stopMonitoringToolStripMenuItem
            // 
            this.stopMonitoringToolStripMenuItem.Name = "stopMonitoringToolStripMenuItem";
            this.stopMonitoringToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.stopMonitoringToolStripMenuItem.Size = new System.Drawing.Size(206, 22);
            this.stopMonitoringToolStripMenuItem.Text = "Stop monitoring";
            this.stopMonitoringToolStripMenuItem.Click += new System.EventHandler(this.StopMonitoringToolStripMenuItem_Click);
            // 
            // hookUnhookDriversToolStripMenuItem
            // 
            this.hookUnhookDriversToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.byPathToolStripMenuItem,
            this.fromListToolStripMenuItem});
            this.hookUnhookDriversToolStripMenuItem.Name = "hookUnhookDriversToolStripMenuItem";
            this.hookUnhookDriversToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.hookUnhookDriversToolStripMenuItem.Text = "Hook / Unhook drivers";
            // 
            // byPathToolStripMenuItem
            // 
            this.byPathToolStripMenuItem.Name = "byPathToolStripMenuItem";
            this.byPathToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.P)));
            this.byPathToolStripMenuItem.Size = new System.Drawing.Size(166, 22);
            this.byPathToolStripMenuItem.Text = "By Path";
            this.byPathToolStripMenuItem.Click += new System.EventHandler(this.ByPathToolStripMenuItem_Click);
            // 
            // fromListToolStripMenuItem
            // 
            this.fromListToolStripMenuItem.Name = "fromListToolStripMenuItem";
            this.fromListToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.fromListToolStripMenuItem.Size = new System.Drawing.Size(166, 22);
            this.fromListToolStripMenuItem.Text = "From List";
            this.fromListToolStripMenuItem.Click += new System.EventHandler(this.FromListToolStripMenuItem_Click);
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
            // ShowIrpBtn
            // 
            this.ShowIrpBtn.Enabled = false;
            this.ShowIrpBtn.Location = new System.Drawing.Point(11, 26);
            this.ShowIrpBtn.Margin = new System.Windows.Forms.Padding(2);
            this.ShowIrpBtn.Name = "ShowIrpBtn";
            this.ShowIrpBtn.Size = new System.Drawing.Size(81, 22);
            this.ShowIrpBtn.TabIndex = 10;
            this.ShowIrpBtn.Text = "Show IRP";
            this.ShowIrpBtn.UseVisualStyleBackColor = true;
            this.ShowIrpBtn.Click += new System.EventHandler(this.ShowIrpBtn_Click);
            // 
            // DumpToFileBtn
            // 
            this.DumpToFileBtn.Enabled = false;
            this.DumpToFileBtn.Location = new System.Drawing.Point(96, 26);
            this.DumpToFileBtn.Margin = new System.Windows.Forms.Padding(2);
            this.DumpToFileBtn.Name = "DumpToFileBtn";
            this.DumpToFileBtn.Size = new System.Drawing.Size(91, 22);
            this.DumpToFileBtn.TabIndex = 11;
            this.DumpToFileBtn.Text = "Dump to File";
            this.DumpToFileBtn.UseVisualStyleBackColor = true;
            this.DumpToFileBtn.Click += new System.EventHandler(this.DumpToFileBtn_Click);
            // 
            // SaveForReplayBtn
            // 
            this.SaveForReplayBtn.Enabled = false;
            this.SaveForReplayBtn.Location = new System.Drawing.Point(191, 26);
            this.SaveForReplayBtn.Margin = new System.Windows.Forms.Padding(2);
            this.SaveForReplayBtn.Name = "SaveForReplayBtn";
            this.SaveForReplayBtn.Size = new System.Drawing.Size(97, 22);
            this.SaveForReplayBtn.TabIndex = 12;
            this.SaveForReplayBtn.Text = "Save for Replay";
            this.SaveForReplayBtn.UseVisualStyleBackColor = true;
            this.SaveForReplayBtn.Click += new System.EventHandler(this.SaveForReplayBtn_Click);
            // 
            // FuzzIrpBtn
            // 
            this.FuzzIrpBtn.Enabled = false;
            this.FuzzIrpBtn.Location = new System.Drawing.Point(292, 26);
            this.FuzzIrpBtn.Margin = new System.Windows.Forms.Padding(2);
            this.FuzzIrpBtn.Name = "FuzzIrpBtn";
            this.FuzzIrpBtn.Size = new System.Drawing.Size(89, 22);
            this.FuzzIrpBtn.TabIndex = 13;
            this.FuzzIrpBtn.Text = "Fuzz IRP";
            this.FuzzIrpBtn.UseVisualStyleBackColor = true;
            this.FuzzIrpBtn.Click += new System.EventHandler(this.FuzzIrpBtn_Click);
            // 
            // CleanIrpDataGridButton
            // 
            this.CleanIrpDataGridButton.Enabled = false;
            this.CleanIrpDataGridButton.Location = new System.Drawing.Point(817, 26);
            this.CleanIrpDataGridButton.Margin = new System.Windows.Forms.Padding(2);
            this.CleanIrpDataGridButton.Name = "CleanIrpDataGridButton";
            this.CleanIrpDataGridButton.Size = new System.Drawing.Size(89, 22);
            this.CleanIrpDataGridButton.TabIndex = 14;
            this.CleanIrpDataGridButton.Text = "Clean IRP";
            this.CleanIrpDataGridButton.UseVisualStyleBackColor = true;
            this.CleanIrpDataGridButton.Click += new System.EventHandler(this.CleanIrpDataGridButton_Click);
            // 
            // IrpMonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(914, 603);
            this.Controls.Add(this.CleanIrpDataGridButton);
            this.Controls.Add(this.FuzzIrpBtn);
            this.Controls.Add(this.SaveForReplayBtn);
            this.Controls.Add(this.DumpToFileBtn);
            this.Controls.Add(this.ShowIrpBtn);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.MenuBar);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "IrpMonitorForm";
            this.Text = "Canadian Fuzzy Bear";
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
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox LogTextBox;
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
        private System.Windows.Forms.Button ShowIrpBtn;
        private System.Windows.Forms.Button DumpToFileBtn;
        private System.Windows.Forms.Button SaveForReplayBtn;
        private System.Windows.Forms.Button FuzzIrpBtn;
        private System.Windows.Forms.ToolStripMenuItem controlToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem driverToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadDriverToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem unloadIrpDumperDriverToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem monitoringToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem startMonitoringToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem stopMonitoringToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem hookUnhookDriversToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem byPathToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fromListToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        public DataGridView IrpDataView;
        private Button CleanIrpDataGridButton;
    }
}

