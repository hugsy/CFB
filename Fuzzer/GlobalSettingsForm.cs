using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;
using Timer = System.Timers.Timer;

namespace Fuzzer
{
    public partial class GlobalSettingsForm : Form
    {

        public GlobalSettingsForm(IrpMonitorForm f)
        {
            InitializeComponent();
            RefreshSettings();
        }


        public void RefreshSettings()
        {
            VerbosityLevelTextBox.Text = Settings.VerbosityLevel.ToString();
            AutoFuzzNewIrpCheckBox.Checked = Settings.AutoFuzzNewIrp;
        }


        private void SendNotificationToStatusBar(string Text)
        {
            SettingsStatusBar.Text = Text;

            Timer aTimer = new Timer(2000);
            aTimer.Elapsed += ResetStatusBarTimedEvent;
            aTimer.AutoReset = true;
            aTimer.Enabled = true;
        }


        private void ResetStatusBarTimedEvent(object sender, ElapsedEventArgs e)
        {
            SettingsStatusBar.Text = "";
        }


        private void VerbosityLevelTextBox_TextChanged(object sender, EventArgs e)
        {
            bool res = System.Int32.TryParse(VerbosityLevelTextBox.Text, out int NewVerbosityLevel);
            if (res)
            {
                Settings.VerbosityLevel = NewVerbosityLevel;
                SendNotificationToStatusBar($"New verbosity level set to {Settings.VerbosityLevel}");
            }           
        }
        

        private void AutoFuzzNewIrpCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            Settings.AutoFuzzNewIrp = AutoFuzzNewIrpCheckBox.Checked;
            SendNotificationToStatusBar($"Automatic IRP fuzzing set to : {Settings.AutoFuzzNewIrp}");
        }


        private void CloseWindowButton_Click(object sender, EventArgs e)
        {
            this.Hide();
        }
    }
}
