using System;
using System.Management;
using System.Data;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Linq;

namespace Fuzzer
{
    public partial class LoadDriverForm : Form
    {
        private DataTable DriverDataTable;
        public List<String> LoadedDrivers;
        private IrpMonitorForm RootForm;

        public LoadDriverForm(IrpMonitorForm f)
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

        private void AddKernelObjectsToDataTable(string ObjectRootPath, string[] IgnoreObjectList = null)
        {
            foreach (string DriverName in EnumerateObjects.EnumerateDirectoryObjects(ObjectRootPath))
            {
                // create a blacklist of drivers to never hook
                if (IgnoreObjectList != null && IgnoreObjectList.Contains( DriverName.ToLower() ))
                {
                    continue;
                }

                string ObjectPath = $"{ObjectRootPath}\\{DriverName:s}";
                DataRow row = DriverDataTable.NewRow();
                row["DriverPath"] = ObjectPath;
                DriverDataTable.Rows.Add(row);
            }
        }


        public void RefreshDriverList()
        {
            DriverDataTable.Clear();

            string[] BlackListedDrivers = new string[] {
                "\\Driver\\IrpDumper"
            };

            string[] BlackListedFSs = new string[] {
            };

            AddKernelObjectsToDataTable("\\Driver", BlackListedDrivers);
            AddKernelObjectsToDataTable("\\FileSystem", BlackListedFSs);
            //AddKernelObjectsToDataTable("\\Device", BlackListedFSs);

            foreach (DataGridViewRow row in LoadedDriverGridView.Rows)
            {
                string DriverName = row.Cells[1].Value.ToString().ToLower();
                bool IsLoaded = LoadedDrivers.Contains(DriverName);
                row.Cells[0].Value = IsLoaded;
            }

            LoadedDriverGridView.Refresh();
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
                var DriverName = row.Cells[1].Value.ToString().ToLower();

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
