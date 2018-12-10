using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Fuzzer
{

    public partial class IrpFilterForm : Form
    {
        private IrpMonitorForm RootForm;
        private IrpDataReader DataReader;
        public List<IrpFilter> IrpFilterList;
        private List<string> ValidColumns;
        private List<string> ValidCondtions;
        private DataTable FilterDataTable;

        public IrpFilterForm(IrpMonitorForm imf, IrpDataReader ird)
        {
            InitializeComponent();

            RootForm = imf;
            DataReader = ird;
            IrpFilterList = new List<IrpFilter>();

            InitComboBoxes();
            InitDataTable();
        }


        private void InitDataTable()
        {
            FilterDataTable = new DataTable("FilterDataTable");
            BindingSource DataBinder = new BindingSource();
            DataBinder.DataSource = FilterDataTable;
            FilterDataGridView.DataSource = DataBinder;

            FilterDataTable.Columns.Add("Column", typeof(string));
            FilterDataTable.Columns.Add("Condition", typeof(string));
            FilterDataTable.Columns.Add("Pattern", typeof(string));
        }


        private void InitComboBoxes()
        {
            ValidColumns = new List<string>();

            // ColumnComboBox
            foreach (var ColumnName in DataReader.IrpDataTable.Columns)
            {
                ValidColumns.Add(ColumnName.ToString());
                ColumnComboBox.Items.Add(ColumnName.ToString());
            }
            ColumnComboBox.DropDownStyle = ComboBoxStyle.DropDownList;


            // ConditionComboBox
            ValidCondtions = new List<string>
            {
                "Equals",
                "Contains",
            };

            foreach (var ConditionName in ValidCondtions)
            {
                ConditionComboBox.Items.Add(ConditionName);
            }

            ConditionComboBox.DropDownStyle = ComboBoxStyle.DropDownList;
        }

        private void AddRuleButton_Click(object sender, EventArgs e)
        {
            string SelectedColumn = (string)ColumnComboBox.SelectedItem;

            if (!ValidColumns.Contains(SelectedColumn))
            {
                return;
            }

            string SelectedCondition = (string)ConditionComboBox.SelectedItem;

            if (!ValidCondtions.Contains(SelectedCondition))
            {
                return;
            }

            string SelectedPattern = PatternTextBox.Text.ToString();

            FilterDataTable.Rows.Add(
                SelectedColumn,
                SelectedCondition,
                SelectedPattern
            );
        }

        private void DeleteRulesButton_Click(object sender, EventArgs e)
        {
            DataGridViewRow SelectedRow = FilterDataGridView.SelectedRows[0];
            FilterDataGridView.Rows.Remove(SelectedRow);
        }

        private void ApplyRulesButton_Click(object sender, EventArgs e)
        {
            IrpFilterList.Clear();

            foreach (DataGridViewRow Row in FilterDataGridView.Rows)
            {
                IrpFilter NewFilter = new IrpFilter(
                    (string)Row.Cells[0].Value,
                    (string)Row.Cells[1].Value,
                    (string)Row.Cells[2].Value
                );

                IrpFilterList.Add(NewFilter);
            }

            this.Hide();
        }
    }


    public class IrpFilter
    {
        private readonly string Column;
        private readonly string Condition;
        private readonly string Pattern;

        public IrpFilter(string Column, string Condition, string Pattern)
        {
            this.Column = Column;
            this.Condition = Condition;
            this.Pattern = Pattern;
        }

        public override string ToString()
        {
            return $"Column:'{Column:s}' Condition:'{Condition:s}' Pattern:'{Pattern:s}'";
        }

        public bool Matches(Irp irp)
        {
            switch (Column)
            {
                case "DeviceName":
                    switch(Condition)
                    {
                        case "Contains": return irp.DeviceName.ToLower().Contains(Pattern.ToLower());
                        case "Equals": return irp.DeviceName.ToLower() == Pattern.ToLower();
                    }
                    break;

                case "DriverName":
                    switch (Condition)
                    {
                        case "Contains": return irp.DriverName.ToLower().Contains(Pattern.ToLower());
                        case "Equals": return irp.DriverName.ToLower() == Pattern.ToLower();
                    }
                    break;
            }

            return false;
        }
    }
}
