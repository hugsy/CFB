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
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.IrpDataView = new System.Windows.Forms.DataGridView();
            this.IrpTimeStamp = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpProcessId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpIoctlCode = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IrpRequestLevel = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Action = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.IrpDataView)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.LogTextBox);
            this.groupBox1.Location = new System.Drawing.Point(26, 42);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(941, 193);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Output";
            this.groupBox1.Enter += new System.EventHandler(this.groupBox1_Enter);
            // 
            // LogTextBox
            // 
            this.LogTextBox.Location = new System.Drawing.Point(18, 18);
            this.LogTextBox.Multiline = true;
            this.LogTextBox.Name = "LogTextBox";
            this.LogTextBox.ReadOnly = true;
            this.LogTextBox.Size = new System.Drawing.Size(904, 162);
            this.LogTextBox.TabIndex = 0;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(995, 42);
            this.button1.Margin = new System.Windows.Forms.Padding(2);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(95, 36);
            this.button1.TabIndex = 1;
            this.button1.Text = "Start Monitoring";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Enabled = false;
            this.button2.Location = new System.Drawing.Point(995, 92);
            this.button2.Margin = new System.Windows.Forms.Padding(2);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(95, 36);
            this.button2.TabIndex = 2;
            this.button2.Text = "Stop Monitoring";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.IrpDataView);
            this.groupBox2.Location = new System.Drawing.Point(26, 254);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox2.Size = new System.Drawing.Size(978, 352);
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
            this.IrpDataView.Margin = new System.Windows.Forms.Padding(2);
            this.IrpDataView.Name = "IrpDataView";
            this.IrpDataView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.AutoSizeToAllHeaders;
            this.IrpDataView.RowTemplate.Height = 28;
            this.IrpDataView.Size = new System.Drawing.Size(974, 335);
            this.IrpDataView.TabIndex = 3;
            this.IrpDataView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.IrpDataView_CellContentClick);

            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1111, 623);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.groupBox1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "Form1";
            this.Text = "Fuzzer Engine for CFB";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.IrpDataView)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox LogTextBox;
        private System.Windows.Forms.DataGridView IrpDataView;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpTimeStamp;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpProcessId;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpIoctlCode;
        private System.Windows.Forms.DataGridViewTextBoxColumn IrpRequestLevel;
        private System.Windows.Forms.DataGridViewTextBoxColumn Action;
    }
}

