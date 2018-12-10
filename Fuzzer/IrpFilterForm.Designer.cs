namespace Fuzzer
{
    partial class IrpFilterForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(IrpFilterForm));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ConditionComboBox = new System.Windows.Forms.ComboBox();
            this.ColumnComboBox = new System.Windows.Forms.ComboBox();
            this.PatternTextBox = new System.Windows.Forms.TextBox();
            this.AddRuleButton = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.FilterDataGridView = new System.Windows.Forms.DataGridView();
            this.DeleteRulesButton = new System.Windows.Forms.Button();
            this.ApplyRulesButton = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.FilterDataGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ConditionComboBox);
            this.groupBox1.Controls.Add(this.ColumnComboBox);
            this.groupBox1.Controls.Add(this.PatternTextBox);
            this.groupBox1.Controls.Add(this.AddRuleButton);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(825, 141);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "New Policy";
            // 
            // ConditionComboBox
            // 
            this.ConditionComboBox.FormattingEnabled = true;
            this.ConditionComboBox.Location = new System.Drawing.Point(234, 81);
            this.ConditionComboBox.Name = "ConditionComboBox";
            this.ConditionComboBox.Size = new System.Drawing.Size(165, 28);
            this.ConditionComboBox.TabIndex = 9;
            // 
            // ColumnComboBox
            // 
            this.ColumnComboBox.FormattingEnabled = true;
            this.ColumnComboBox.Location = new System.Drawing.Point(19, 81);
            this.ColumnComboBox.Name = "ColumnComboBox";
            this.ColumnComboBox.Size = new System.Drawing.Size(179, 28);
            this.ColumnComboBox.TabIndex = 8;
            // 
            // PatternTextBox
            // 
            this.PatternTextBox.Location = new System.Drawing.Point(430, 78);
            this.PatternTextBox.Name = "PatternTextBox";
            this.PatternTextBox.Size = new System.Drawing.Size(220, 26);
            this.PatternTextBox.TabIndex = 7;
            // 
            // AddRuleButton
            // 
            this.AddRuleButton.Location = new System.Drawing.Point(691, 55);
            this.AddRuleButton.Name = "AddRuleButton";
            this.AddRuleButton.Size = new System.Drawing.Size(116, 54);
            this.AddRuleButton.TabIndex = 6;
            this.AddRuleButton.Text = "Add Rule";
            this.AddRuleButton.UseVisualStyleBackColor = true;
            this.AddRuleButton.Click += new System.EventHandler(this.AddRuleButton_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(426, 55);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(61, 20);
            this.label3.TabIndex = 5;
            this.label3.Text = "Pattern";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(230, 55);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(76, 20);
            this.label2.TabIndex = 4;
            this.label2.Text = "Condition";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 55);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(63, 20);
            this.label1.TabIndex = 3;
            this.label1.Text = "Column";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.FilterDataGridView);
            this.groupBox2.Location = new System.Drawing.Point(12, 181);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(825, 316);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Current Filtering Policy";
            // 
            // FilterDataGridView
            // 
            this.FilterDataGridView.AllowUserToAddRows = false;
            this.FilterDataGridView.AllowUserToDeleteRows = false;
            this.FilterDataGridView.AllowUserToResizeColumns = false;
            this.FilterDataGridView.AllowUserToResizeRows = false;
            this.FilterDataGridView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.ColumnHeader;
            this.FilterDataGridView.AutoSizeRowsMode = System.Windows.Forms.DataGridViewAutoSizeRowsMode.AllHeaders;
            this.FilterDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.FilterDataGridView.Location = new System.Drawing.Point(19, 35);
            this.FilterDataGridView.Name = "FilterDataGridView";
            this.FilterDataGridView.ReadOnly = true;
            this.FilterDataGridView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.DisableResizing;
            this.FilterDataGridView.RowTemplate.Height = 28;
            this.FilterDataGridView.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.FilterDataGridView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.FilterDataGridView.Size = new System.Drawing.Size(788, 272);
            this.FilterDataGridView.TabIndex = 0;
            // 
            // DeleteRulesButton
            // 
            this.DeleteRulesButton.Location = new System.Drawing.Point(70, 503);
            this.DeleteRulesButton.Name = "DeleteRulesButton";
            this.DeleteRulesButton.Size = new System.Drawing.Size(191, 39);
            this.DeleteRulesButton.TabIndex = 0;
            this.DeleteRulesButton.Text = "Delete Selected Rule(s)";
            this.DeleteRulesButton.UseVisualStyleBackColor = true;
            this.DeleteRulesButton.Click += new System.EventHandler(this.DeleteRulesButton_Click);
            // 
            // ApplyRulesButton
            // 
            this.ApplyRulesButton.Location = new System.Drawing.Point(519, 503);
            this.ApplyRulesButton.Name = "ApplyRulesButton";
            this.ApplyRulesButton.Size = new System.Drawing.Size(188, 39);
            this.ApplyRulesButton.TabIndex = 2;
            this.ApplyRulesButton.Text = "Apply Rules";
            this.ApplyRulesButton.UseVisualStyleBackColor = true;
            this.ApplyRulesButton.Click += new System.EventHandler(this.ApplyRulesButton_Click);
            // 
            // IrpFilterForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(846, 549);
            this.Controls.Add(this.ApplyRulesButton);
            this.Controls.Add(this.DeleteRulesButton);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "IrpFilterForm";
            this.Text = "Define Filtering Rules";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.FilterDataGridView)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button DeleteRulesButton;
        private System.Windows.Forms.Button ApplyRulesButton;
        private System.Windows.Forms.Button AddRuleButton;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox PatternTextBox;
        private System.Windows.Forms.ComboBox ConditionComboBox;
        private System.Windows.Forms.ComboBox ColumnComboBox;
        private System.Windows.Forms.DataGridView FilterDataGridView;

    }
}