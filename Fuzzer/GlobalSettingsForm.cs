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
            int NewVerbosityLevel = 1;
            var text = VerbosityLevelTextBox.Text.ToUpper();
            switch (text)
            {
                case "DEBUG":
                    NewVerbosityLevel = 0;
                    break;
                case "INFO":
                    NewVerbosityLevel = 1;
                    break;
                case "WARNING":
                    NewVerbosityLevel = 2;
                    break;
                case "ERROR":
                    NewVerbosityLevel = 3;
                    break;
                case "CRITICAL":
                    NewVerbosityLevel = 4;
                    break;
            }

            Settings.VerbosityLevel = NewVerbosityLevel;
            SendNotificationToStatusBar($"New verbosity level set to {Settings.VerbosityLevel}");
        }
        

        private void AutoFuzzNewIrpCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            //Settings.AutoFuzzNewIrp = AutoFuzzNewIrpCheckBox.Checked;
            //SendNotificationToStatusBar($"Automatic IRP fuzzing set to : {Settings.AutoFuzzNewIrp}");
        }


        private void CloseWindowButton_Click(object sender, EventArgs e)
        {
            this.Hide();
        }


        private void ServerUriTextBox_TextChanged(object sender, EventArgs e)
        {
            Settings.BrokerUri = ServerUriTextBox.Text.ToString();
        }
    }
}
