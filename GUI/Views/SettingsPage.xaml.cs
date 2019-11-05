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
        private ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;


        public SettingsPage()
        {
            this.InitializeComponent();
        }


        private void settingIrpBrokerLocationTextBox_Changed(object sender, RoutedEventArgs e)
        {
            var location = settingIrpBrokerLocationTextBox.Text.ToLower();
            if (IsValidLocationFormat(location))
            {
                localSettings.Values["IrpBrokerLocation"] = location;
                settingIrpBrokerLocationTextBox.BorderBrush = new SolidColorBrush(Windows.UI.Colors.Green);
            }
            else
            {
                settingIrpBrokerLocationTextBox.BorderBrush = new SolidColorBrush(Windows.UI.Colors.Red);
            }
        }


        private bool IsValidLocationFormat(string location)
        {
            try
            {
                var uri = new Uri(location);

                if (uri.Scheme != "file")
                    return false;

                return true;
            }
            catch(Exception)
            {
            }
            
            return false;
        }


        private void onEnableAutoFuzzSetting_Click(object sender, RoutedEventArgs e)
        {
            localSettings.Values["EnableAutoFuzz"] = enableAutoFuzzCheckBox.IsOn;
            autoFuzzStrategiesPanel.Visibility = enableAutoFuzzCheckBox.IsOn ? Visibility.Visible : Visibility.Collapsed;
        }
    }
}
