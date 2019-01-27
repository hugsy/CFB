namespace Fuzzer
{
    partial class GlobalSettingsForm
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
            this.GeneralSettingsGroupBox = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.FuzzingGroupBox = new System.Windows.Forms.GroupBox();
            this.CloseWindowButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.VerbosityLevelTextBox = new System.Windows.Forms.TextBox();
            this.AutoFuzzNewIrpCheckBox = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.SettingsStatusBar = new System.Windows.Forms.ToolStripStatusLabel();
            this.GeneralSettingsGroupBox.SuspendLayout();
            this.FuzzingGroupBox.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // GeneralSettingsGroupBox
            // 
            this.GeneralSettingsGroupBox.Controls.Add(this.VerbosityLevelTextBox);
            this.GeneralSettingsGroupBox.Controls.Add(this.label1);
            this.GeneralSettingsGroupBox.Location = new System.Drawing.Point(12, 12);
            this.GeneralSettingsGroupBox.Name = "GeneralSettingsGroupBox";
            this.GeneralSettingsGroupBox.Size = new System.Drawing.Size(791, 209);
            this.GeneralSettingsGroupBox.TabIndex = 2;
            this.GeneralSettingsGroupBox.TabStop = false;
            this.GeneralSettingsGroupBox.Text = "General";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(35, 49);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(116, 20);
            this.label1.TabIndex = 0;
            this.label1.Text = "Verbosity Level";
            // 
            // FuzzingGroupBox
            // 
            this.FuzzingGroupBox.Controls.Add(this.label3);
            this.FuzzingGroupBox.Controls.Add(this.AutoFuzzNewIrpCheckBox);
            this.FuzzingGroupBox.Controls.Add(this.label2);
            this.FuzzingGroupBox.Location = new System.Drawing.Point(12, 227);
            this.FuzzingGroupBox.Name = "FuzzingGroupBox";
            this.FuzzingGroupBox.Size = new System.Drawing.Size(791, 207);
            this.FuzzingGroupBox.TabIndex = 3;
            this.FuzzingGroupBox.TabStop = false;
            this.FuzzingGroupBox.Text = "Fuzzing";
            // 
            // CloseWindowButton
            // 
            this.CloseWindowButton.Location = new System.Drawing.Point(347, 440);
            this.CloseWindowButton.Name = "CloseWindowButton";
            this.CloseWindowButton.Size = new System.Drawing.Size(148, 36);
            this.CloseWindowButton.TabIndex = 1;
            this.CloseWindowButton.Text = "Close Window";
            this.CloseWindowButton.UseVisualStyleBackColor = true;
            this.CloseWindowButton.Click += new System.EventHandler(this.CloseWindowButton_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(35, 51);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(201, 20);
            this.label2.TabIndex = 1;
            this.label2.Text = "Automatically fuzz new IRP";
            // 
            // VerbosityLevelTextBox
            // 
            this.VerbosityLevelTextBox.Location = new System.Drawing.Point(575, 43);
            this.VerbosityLevelTextBox.Name = "VerbosityLevelTextBox";
            this.VerbosityLevelTextBox.Size = new System.Drawing.Size(194, 26);
            this.VerbosityLevelTextBox.TabIndex = 1;
            this.VerbosityLevelTextBox.TextChanged += new System.EventHandler(this.VerbosityLevelTextBox_TextChanged);
            // 
            // AutoFuzzNewIrpCheckBox
            // 
            this.AutoFuzzNewIrpCheckBox.AutoSize = true;
            this.AutoFuzzNewIrpCheckBox.Location = new System.Drawing.Point(575, 50);
            this.AutoFuzzNewIrpCheckBox.Name = "AutoFuzzNewIrpCheckBox";
            this.AutoFuzzNewIrpCheckBox.Size = new System.Drawing.Size(22, 21);
            this.AutoFuzzNewIrpCheckBox.TabIndex = 2;
            this.AutoFuzzNewIrpCheckBox.UseVisualStyleBackColor = true;
            this.AutoFuzzNewIrpCheckBox.CheckedChanged += new System.EventHandler(this.AutoFuzzNewIrpCheckBox_CheckedChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(35, 85);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(192, 20);
            this.label3.TabIndex = 3;
            this.label3.Text = "Automatic fuzzing policies";
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.SettingsStatusBar});
            this.statusStrip1.Location = new System.Drawing.Point(0, 510);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(834, 30);
            this.statusStrip1.TabIndex = 4;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // SettingsStatusBar
            // 
            this.SettingsStatusBar.Name = "SettingsStatusBar";
            this.SettingsStatusBar.Size = new System.Drawing.Size(179, 25);
            this.SettingsStatusBar.Text = "toolStripStatusLabel1";
            // 
            // GlobalSettingsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(834, 540);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.FuzzingGroupBox);
            this.Controls.Add(this.GeneralSettingsGroupBox);
            this.Controls.Add(this.CloseWindowButton);
            this.Name = "GlobalSettingsForm";
            this.Text = "Settings";
            this.GeneralSettingsGroupBox.ResumeLayout(false);
            this.GeneralSettingsGroupBox.PerformLayout();
            this.FuzzingGroupBox.ResumeLayout(false);
            this.FuzzingGroupBox.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.GroupBox GeneralSettingsGroupBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox FuzzingGroupBox;
        private System.Windows.Forms.Button CloseWindowButton;
        private System.Windows.Forms.TextBox VerbosityLevelTextBox;
        private System.Windows.Forms.CheckBox AutoFuzzNewIrpCheckBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel SettingsStatusBar;
    }
}