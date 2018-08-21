using System;
using System.Management;
using System.Data;
using System.Windows.Forms;
using System.Collections.Generic;

namespace Fuzzer
{
    public partial class LoadDriverForm : Form
    {
        private DataTable DriverDataTable;
        private List<String> LoadedDrivers;

        public LoadDriverForm()
        {
            InitializeComponent();
            try
            {
                this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;

            LoadedDrivers = new List<String>();
            DriverDataTable = new DataTable();
            DriverDataTable.Columns.Add("ImageBase", typeof(String));
            DriverDataTable.Columns.Add("Name", typeof(String));

            LoadedDriverGridView.DataSource = DriverDataTable;
            var CheckboxColumn = new DataGridViewCheckBoxColumn();
            CheckboxColumn.HeaderText = "Hooked ?";
            LoadedDriverGridView.Columns.Insert(0, CheckboxColumn);
            LoadedDriverGridView.Columns["ImageBase"].ReadOnly = true;
            LoadedDriverGridView.Columns["Name"].ReadOnly = true;


                RefreshDriverList();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }

        private void RefreshDriverList()
        {
            DriverDataTable.Clear();
            
            foreach (Tuple<UIntPtr, string> kvp in EnumerateDrivers.GetAllDrivers() )
            {
                DataRow row = DriverDataTable.NewRow();
                row["ImageBase"] = kvp.Item1.ToUInt64().ToString("x");
                row["Name"] = kvp.Item2;
                DriverDataTable.Rows.Add(row);
            }


            foreach (DataGridViewRow row in LoadedDriverGridView.Rows)
            {
                row.Cells[0].Value = LoadedDrivers.Contains(row.Cells[2].Value.ToString());
            }

        }


        private void CloseBtn_Click(object sender, EventArgs e)
        {
            this.Hide();
        }

        private void RefreshBtn_Click(object sender, EventArgs e)
        {
            RefreshDriverList();
        }
    }
}
