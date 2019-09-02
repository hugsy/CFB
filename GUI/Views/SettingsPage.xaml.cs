using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Storage;



namespace GUI.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SettingsPage : Page
    {

        public string settingIrpBrokerLocation = "tcp://localhost:1337";
        private ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;

        public SettingsPage()
        {
            this.InitializeComponent();
        }

        public bool IsBrokerLocationValid()
        {
            return true;
        }

        private void settingIrpBrokerLocationTextBox_Changed(object sender, RoutedEventArgs e)
        {
            var new_value = settingIrpBrokerLocationTextBox.Text;
            if (new_value.StartsWith("tcp://"))
            {
                // todo make better checks
                settingIrpBrokerLocation = new_value;
                localSettings.Values["IrpBrokerLocation"] = new_value;

                settingIrpBrokerLocationTextBox.BorderBrush = new SolidColorBrush(Windows.UI.Colors.Green);
            }
            else
            {
                settingIrpBrokerLocationTextBox.BorderBrush = new SolidColorBrush(Windows.UI.Colors.Red);
            }
        }

        private void onEnableAutoFuzzSetting_Click(object sender, RoutedEventArgs e)
        {
            localSettings.Values["EnableAutoFuzz"] = enableAutoFuzzCheckBox.IsChecked;
        }
    }
}
