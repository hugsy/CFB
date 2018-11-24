using System.Windows.Forms;

namespace Fuzzer
{
    partial class LoadDriverForm
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
            this.LoadedDriverGridView = new System.Windows.Forms.DataGridView();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.CloseBtn = new System.Windows.Forms.Button();
            this.RefreshBtn = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.LoadedDriverGridView)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // LoadedDriverGridView
            // 
            this.LoadedDriverGridView.AllowUserToAddRows = false;
            this.LoadedDriverGridView.AllowUserToDeleteRows = false;
            this.LoadedDriverGridView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.LoadedDriverGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.LoadedDriverGridView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.LoadedDriverGridView.Location = new System.Drawing.Point(2, 15);
            this.LoadedDriverGridView.Margin = new System.Windows.Forms.Padding(2);
            this.LoadedDriverGridView.Name = "LoadedDriverGridView";
            this.LoadedDriverGridView.RowTemplate.Height = 28;
            this.LoadedDriverGridView.Size = new System.Drawing.Size(570, 314);
            this.LoadedDriverGridView.TabIndex = 0;
            this.LoadedDriverGridView.SelectionMode = DataGridViewSelectionMode.FullRowSelect;

            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.LoadedDriverGridView);
            this.groupBox1.Location = new System.Drawing.Point(8, 8);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(574, 331);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Select the driver(s) to be hooked";
            // 
            // CloseBtn
            // 
            this.CloseBtn.Location = new System.Drawing.Point(151, 343);
            this.CloseBtn.Margin = new System.Windows.Forms.Padding(2);
            this.CloseBtn.Name = "CloseBtn";
            this.CloseBtn.Size = new System.Drawing.Size(86, 37);
            this.CloseBtn.TabIndex = 2;
            this.CloseBtn.Text = "Close";
            this.CloseBtn.UseVisualStyleBackColor = true;
            this.CloseBtn.Click += new System.EventHandler(this.CloseBtn_Click);
            // 
            // RefreshBtn
            // 
            this.RefreshBtn.Location = new System.Drawing.Point(316, 343);
            this.RefreshBtn.Margin = new System.Windows.Forms.Padding(2);
            this.RefreshBtn.Name = "RefreshBtn";
            this.RefreshBtn.Size = new System.Drawing.Size(86, 37);
            this.RefreshBtn.TabIndex = 3;
            this.RefreshBtn.Text = "Refresh List";
            this.RefreshBtn.UseVisualStyleBackColor = true;
            this.RefreshBtn.Click += new System.EventHandler(this.RefreshBtn_Click);
            // 
            // LoadDriverForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(590, 388);
            this.Controls.Add(this.RefreshBtn);
            this.Controls.Add(this.CloseBtn);
            this.Controls.Add(this.groupBox1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "LoadDriverForm";
            this.Text = "Driver selection menu";
            ((System.ComponentModel.ISupportInitialize)(this.LoadedDriverGridView)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView LoadedDriverGridView;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button CloseBtn;
        private System.Windows.Forms.Button RefreshBtn;
    }
}