using System;
using System.Management;
using System.Collections.Generic;
using System.Data;
using System.Windows.Forms;


namespace Fuzzer
{
    public partial class LoadDriverForm : Form
    {
        private DataTable DriverDataTable;

        public LoadDriverForm()
        {
            InitializeComponent();
            DriverDataTable = new DataTable();
            RefreshDriverList();
            LoadedDriverGridView.DataSource = DriverDataTable;
        }

        private void RefreshDriverList()
        {

            DriverDataTable.Clear();

            DriverDataTable.Columns.Add("Name", typeof(String));
            DriverDataTable.Columns.Add("Path", typeof(String));

            SelectQuery WmiQuery = new SelectQuery("Win32_Service");
            using (ManagementObjectSearcher cursor = new ManagementObjectSearcher(WmiQuery))
            {
                foreach (ManagementObject service in cursor.Get())
                {
                    DriverDataTable.Rows.Add(
                        service["Name"],
                        service["Path"]
                        );
                }
            }

            DataGridViewCheckBoxColumn SelectedColumn = new DataGridViewCheckBoxColumn();
            LoadedDriverGridView.Columns.Insert(0, SelectedColumn);
            foreach (DataGridViewRow row in LoadedDriverGridView.Rows)
            {
                if(IsLoaded(row.Cells[2].Value.ToString()))
                    row.Cells[0].Value = true;
                else
                    row.Cells[0].Value = false;
            }

        }

        private bool IsLoaded(String value)
        {
            return false; // todo
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
