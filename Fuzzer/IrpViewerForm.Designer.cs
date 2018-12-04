namespace Fuzzer
{
    partial class IrpViewerForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(IrpViewerForm));
            this.IrpDetails = new System.Windows.Forms.GroupBox();
            this.IrpIrqlTextBox = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.IrpIoctlCodeTextBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.IrpProcessNameTextBox = new System.Windows.Forms.TextBox();
            this.IrpTimestampTextBox = new System.Windows.Forms.TextBox();
            this.IrpDeviceNameTextBox = new System.Windows.Forms.TextBox();
            this.IrpDriverNameTextBox = new System.Windows.Forms.TextBox();
            this.IrpIndexTextBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.IrpHexdump = new System.Windows.Forms.GroupBox();
            this.IrpBodyHexdumpTextBox = new System.Windows.Forms.RichTextBox();
            this.IrpDetails.SuspendLayout();
            this.IrpHexdump.SuspendLayout();
            this.SuspendLayout();
            // 
            // IrpDetails
            // 
            this.IrpDetails.Controls.Add(this.IrpIrqlTextBox);
            this.IrpDetails.Controls.Add(this.label7);
            this.IrpDetails.Controls.Add(this.IrpIoctlCodeTextBox);
            this.IrpDetails.Controls.Add(this.label6);
            this.IrpDetails.Controls.Add(this.IrpProcessNameTextBox);
            this.IrpDetails.Controls.Add(this.IrpTimestampTextBox);
            this.IrpDetails.Controls.Add(this.IrpDeviceNameTextBox);
            this.IrpDetails.Controls.Add(this.IrpDriverNameTextBox);
            this.IrpDetails.Controls.Add(this.IrpIndexTextBox);
            this.IrpDetails.Controls.Add(this.label4);
            this.IrpDetails.Controls.Add(this.label3);
            this.IrpDetails.Controls.Add(this.label2);
            this.IrpDetails.Controls.Add(this.label1);
            this.IrpDetails.Controls.Add(this.label5);
            this.IrpDetails.Location = new System.Drawing.Point(18, 18);
            this.IrpDetails.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpDetails.Name = "IrpDetails";
            this.IrpDetails.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpDetails.Size = new System.Drawing.Size(849, 286);
            this.IrpDetails.TabIndex = 0;
            this.IrpDetails.TabStop = false;
            this.IrpDetails.Text = "Details";
            // 
            // IrpIrqlTextBox
            // 
            this.IrpIrqlTextBox.Location = new System.Drawing.Point(376, 245);
            this.IrpIrqlTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpIrqlTextBox.Name = "IrpIrqlTextBox";
            this.IrpIrqlTextBox.ReadOnly = true;
            this.IrpIrqlTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpIrqlTextBox.TabIndex = 13;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(51, 245);
            this.label7.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(315, 20);
            this.label7.TabIndex = 12;
            this.label7.Text = "IRQ Level.........................";
            // 
            // IrpIoctlCodeTextBox
            // 
            this.IrpIoctlCodeTextBox.Location = new System.Drawing.Point(376, 209);
            this.IrpIoctlCodeTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpIoctlCodeTextBox.Name = "IrpIoctlCodeTextBox";
            this.IrpIoctlCodeTextBox.ReadOnly = true;
            this.IrpIoctlCodeTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpIoctlCodeTextBox.TabIndex = 11;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(51, 209);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(315, 20);
            this.label6.TabIndex = 10;
            this.label6.Text = "IOCTL Code........................";
            this.label6.UseWaitCursor = true;
            // 
            // IrpProcessNameTextBox
            // 
            this.IrpProcessNameTextBox.Location = new System.Drawing.Point(376, 174);
            this.IrpProcessNameTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpProcessNameTextBox.Name = "IrpProcessNameTextBox";
            this.IrpProcessNameTextBox.ReadOnly = true;
            this.IrpProcessNameTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpProcessNameTextBox.TabIndex = 9;
            // 
            // IrpTimestampTextBox
            // 
            this.IrpTimestampTextBox.Location = new System.Drawing.Point(376, 135);
            this.IrpTimestampTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpTimestampTextBox.Name = "IrpTimestampTextBox";
            this.IrpTimestampTextBox.ReadOnly = true;
            this.IrpTimestampTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpTimestampTextBox.TabIndex = 8;
            // 
            // IrpDeviceNameTextBox
            // 
            this.IrpDeviceNameTextBox.Location = new System.Drawing.Point(376, 100);
            this.IrpDeviceNameTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpDeviceNameTextBox.Name = "IrpDeviceNameTextBox";
            this.IrpDeviceNameTextBox.ReadOnly = true;
            this.IrpDeviceNameTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpDeviceNameTextBox.TabIndex = 7;
            // 
            // IrpDriverNameTextBox
            // 
            this.IrpDriverNameTextBox.Location = new System.Drawing.Point(376, 63);
            this.IrpDriverNameTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpDriverNameTextBox.Name = "IrpDriverNameTextBox";
            this.IrpDriverNameTextBox.ReadOnly = true;
            this.IrpDriverNameTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpDriverNameTextBox.TabIndex = 6;
            // 
            // IrpIndexTextBox
            // 
            this.IrpIndexTextBox.Location = new System.Drawing.Point(376, 29);
            this.IrpIndexTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpIndexTextBox.Name = "IrpIndexTextBox";
            this.IrpIndexTextBox.ReadOnly = true;
            this.IrpIndexTextBox.Size = new System.Drawing.Size(426, 26);
            this.IrpIndexTextBox.TabIndex = 5;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(51, 174);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(315, 20);
            this.label4.TabIndex = 4;
            this.label4.Text = "Process...........................";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(51, 135);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(315, 20);
            this.label3.TabIndex = 3;
            this.label3.Text = "Timestamp.........................";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(51, 98);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(315, 20);
            this.label2.TabIndex = 2;
            this.label2.Text = "Device............................";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(51, 63);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(315, 20);
            this.label1.TabIndex = 1;
            this.label1.Text = "Driver............................";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(51, 29);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(315, 20);
            this.label5.TabIndex = 0;
            this.label5.Text = "Index ............................";
            // 
            // IrpHexdump
            // 
            this.IrpHexdump.Controls.Add(this.IrpBodyHexdumpTextBox);
            this.IrpHexdump.Location = new System.Drawing.Point(18, 314);
            this.IrpHexdump.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpHexdump.Name = "IrpHexdump";
            this.IrpHexdump.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpHexdump.Size = new System.Drawing.Size(849, 426);
            this.IrpHexdump.TabIndex = 1;
            this.IrpHexdump.TabStop = false;
            this.IrpHexdump.Text = "Hexdump";
            // 
            // IrpBodyHexdumpTextBox
            // 
            this.IrpBodyHexdumpTextBox.Font = new System.Drawing.Font("Consolas", 8F);
            this.IrpBodyHexdumpTextBox.Location = new System.Drawing.Point(4, 25);
            this.IrpBodyHexdumpTextBox.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.IrpBodyHexdumpTextBox.Name = "IrpBodyHexdumpTextBox";
            this.IrpBodyHexdumpTextBox.ReadOnly = true;
            this.IrpBodyHexdumpTextBox.Size = new System.Drawing.Size(834, 390);
            this.IrpBodyHexdumpTextBox.TabIndex = 0;
            this.IrpBodyHexdumpTextBox.Text = "";
            // 
            // IrpViewerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(874, 758);
            this.Controls.Add(this.IrpHexdump);
            this.Controls.Add(this.IrpDetails);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "IrpViewerForm";
            this.Text = "Details for IRP";
            this.IrpDetails.ResumeLayout(false);
            this.IrpDetails.PerformLayout();
            this.IrpHexdump.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox IrpDetails;
        private System.Windows.Forms.GroupBox IrpHexdump;
        private System.Windows.Forms.RichTextBox IrpBodyHexdumpTextBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox IrpIoctlCodeTextBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox IrpProcessNameTextBox;
        private System.Windows.Forms.TextBox IrpTimestampTextBox;
        private System.Windows.Forms.TextBox IrpDeviceNameTextBox;
        private System.Windows.Forms.TextBox IrpDriverNameTextBox;
        private System.Windows.Forms.TextBox IrpIndexTextBox;
        private System.Windows.Forms.TextBox IrpIrqlTextBox;
        private System.Windows.Forms.Label label7;
    }
}