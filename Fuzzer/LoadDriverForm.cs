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
        public List<String> LoadedDrivers;
        private Form1 RootForm;

        public LoadDriverForm(Form1 f)
        {
            InitializeComponent();

            RootForm = f;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;

            LoadedDrivers = new List<String>();
            DriverDataTable = new DataTable();
            DriverDataTable.Columns.Add("DriverPath", typeof(String));

            LoadedDriverGridView.DataSource = DriverDataTable;
            var CheckboxColumn = new DataGridViewCheckBoxColumn()
            {
                HeaderText = "Hooked ?"
            };

            LoadedDriverGridView.Columns.Insert(0, CheckboxColumn);
            LoadedDriverGridView.Columns["DriverPath"].ReadOnly = true;

            RefreshDriverList();
        }

        public void RefreshDriverList()
        {
            DriverDataTable.Clear();

            string RootPath = "\\driver";

            foreach (var DevicePath in EnumerateDrivers.EnumerateDirectoryObjects(RootPath))
            {
                // create a blacklist of drivers to never hook
                if (DevicePath == "IrpDumper")
                {
                    continue;
                }

                DataRow row = DriverDataTable.NewRow();
                row["DriverPath"] = RootPath + "\\" + DevicePath;
                DriverDataTable.Rows.Add(row);
            }

            foreach (DataGridViewRow row in LoadedDriverGridView.Rows)
            {
                row.Cells[0].Value = LoadedDrivers.Contains(row.Cells[1].Value.ToString());
            }

        }


        private void CloseBtn_Click(object sender, EventArgs e)
        {
            //
            // check for changes
            //
            List<DataGridViewRow> rows_with_checked_column = new List<DataGridViewRow>();
            foreach (DataGridViewRow row in LoadedDriverGridView.Rows)
            {
                var IsTicked = Convert.ToBoolean(row.Cells[0].Value);
                var DriverName = row.Cells[1].Value.ToString();

                //
                // unhook driver ?
                //
                if (!IsTicked && LoadedDrivers.Contains(DriverName))
                {
                    RootForm.Log(String.Format("Unhooking '{0:s}'", DriverName));
                    if (!Core.UnhookDriver(DriverName))
                    {
                        RootForm.Log(String.Format("Failed to unhook '{0:s}'", DriverName));
                    }
                    else
                    {
                        LoadedDrivers.Remove(DriverName);
                        RootForm.Log(String.Format("Driver object '{0:s}' is now unhooked.", DriverName));
                    }
                    continue;
                }

                //
                // hook driver ?
                //
                if (IsTicked && !LoadedDrivers.Contains(DriverName))
                {
                    RootForm.Log(String.Format("Hooking '{0:s}'", DriverName));
                    if (!Core.HookDriver(DriverName))
                    {
                        RootForm.Log(String.Format("Failed to hook '{0:s}'", DriverName));
                    }
                    else
                    {
                        LoadedDrivers.Add(DriverName);
                        RootForm.Log(String.Format("Driver object '{0:s}' is now hooked.", DriverName));
                    }
                    continue;
                }
            }

            this.Hide();
        }

        private void RefreshBtn_Click(object sender, EventArgs e)
        {
            RefreshDriverList();
        }
    }
}
