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
using GUI.Models;

namespace GUI.Views
{

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

                if (uri.Scheme != "tcp")
                    return false;

                if (uri.Port < 0)
                    return false;
            }
            catch(Exception)
            {
                return false;
            }

            return true;
        }



        private void settingBrokerPollDelay_Changed(object sender, RoutedEventArgs e)
        {
            try
            {
                var val = Convert.ToDouble(settingBrokerPollDelay.Text.ToString());
                localSettings.Values[IrpDumper.IrpDumperPollDelayKey] = val;
            }
            catch
            {
                // fallback to default
                localSettings.Values[IrpDumper.IrpDumperPollDelayKey] = IrpDumper.IrpDumperDefaultProbeValue;
            }
        }
    }
}
