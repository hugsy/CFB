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
using Windows.System;
using System.Threading.Tasks;

using GUI.Models;
using Windows.Storage;
using Windows.UI.Popups;
using GUI.Helpers;

namespace GUI
{

    public sealed partial class AppShell : Page
    {
        public readonly string MonitoredIrpsListLabel      = "Monitored IRPs";
        public readonly string OpenIrpDbLabel              = "Load IRPs from file";
        public readonly string SaveIrpDbLabel              = "Save IRPs to file";
        public readonly string ManageDriversLabel          = "Add/remove drivers";
        public readonly string ReplayIrpLabel              = "Forge custom IRP";
        public readonly string AboutLabel                  = "About CFB";

        public readonly string StartMonitoringLabel        = "Start Monitoring IRPs";
        public readonly string StopMonitoringLabel         = "Stop Monitoring IRPs";
        public readonly string ClearGridLabel              = "Clear all intercepted IRPs";

        public readonly string ConnectedStatusLabel        = "Connected, click to disconnect...";
        public readonly string DisconnectedStatusLabel     = "Disconnected, click to connect...";

        public readonly string GlobalState_Connected       = "Connected";
        public readonly string GlobalState_Disconnected    = "Disconnected";
        public readonly string GlobalState_Capturing       = "Capturing...";


        public Frame AppFrame => frame;

        public AppShell()
        {
            this.InitializeComponent();
        }


        /// <summary>
        /// The AppShell NavigationView "on tap" dispatcher
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void NavView_OnItemTapped(NavigationView sender, NavigationViewItemInvokedEventArgs args)
        {
            var label = args.InvokedItem as string;

            // default page should be the irp data grid
            var targetPage = typeof(Views.MonitoredIrpsPage); 

            if (args.IsSettingsInvoked)
            {
                targetPage = typeof(Views.SettingsPage);
            }
            else
            {

                if (label == MonitoredIrpsListLabel)
                    targetPage = typeof(Views.MonitoredIrpsPage);

                else if (label == AboutLabel)
                    targetPage = typeof(Views.AboutPage);

                else if (label == OpenIrpDbLabel)
                    targetPage = typeof(Views.LoadIrpsFromFilePage);

                else if (label == SaveIrpDbLabel)
                    targetPage = typeof(Views.SaveIrpsToFilePage);

                else if (label == ManageDriversLabel)
                    targetPage = typeof(Views.DriverListPage);

                else if (label == ReplayIrpLabel)
                    targetPage = typeof(Views.ReplayIrpPage);

                //
                // TODO: Add other pages
                //
            }

            NavView.IsPaneOpen = false;
            AppFrame.Navigate(targetPage);
        }


        /// <summary>
        /// When navigating to a new page in the root frame, keep track of the previous page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnNavigatingToPage(object sender, NavigatingCancelEventArgs e)
        {
            if (e.NavigationMode == NavigationMode.Back)
            {
                if (e.SourcePageType == typeof(Views.SettingsPage))
                    NavView.SelectedItem = NavView.SettingsItem;

                else if (e.SourcePageType == typeof(Views.MonitoredIrpsPage))
                    NavView.SelectedItem = MonitoredIrpsMenuItem;

                else if (e.SourcePageType == typeof(Views.AboutPage))
                    NavView.SelectedItem = AboutMenuItem;

                else if (e.SourcePageType == typeof(Views.LoadIrpsFromFilePage))
                    NavView.SelectedItem = OpenIrpDbMenuItem;

                else if (e.SourcePageType == typeof(Views.SaveIrpsToFilePage))
                    NavView.SelectedItem = SaveIrpDbMenuItem;

                else if (e.SourcePageType == typeof(Views.DriverListPage))
                    NavView.SelectedItem = AddRemoveDriversMenuItem;

                else if (e.SourcePageType == typeof(Views.ReplayIrpPage))
                    NavView.SelectedItem = ReplayIrpMenuItem;

                //
                // TODO add other pages
                //
            }
        }


        /// <summary>
        /// Open the browser page to report a bug
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void ReportBug_Tapped(object sender, TappedRoutedEventArgs e) =>
            await Launcher.LaunchUriAsync(new Uri(Constants.ProjectIssueUrl));


        /// <summary>
        /// Returns to the previous page if possible
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void NavView_BackRequested(NavigationView sender, NavigationViewBackRequestedEventArgs args)
        {
            if (AppFrame.CanGoBack)
            {
                AppFrame.GoBack();
            }
        }

        public void UpdateGlobalState(string message)
        {
            CurrentStateInfoLbl.Text = message;
        }


        private async void ToggleConnectButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!App.BrokerSession.IsConnected)
                    TryConnect();
                else
                    TryDisconnect();
            }
            catch (Exception ex)
            {
                var brokerPathSetting = ApplicationData.Current.LocalSettings.Values["IrpBrokerLocation"].ToString();
                var dialog = new MessageDialog($"An error occured while trying to open/close connection with '{brokerPathSetting}'. " +
                    $"Reason:\n{ex.Message}", 
                    "Connection error"
                );
                await dialog.ShowAsync();
            }
        }      

        private async void TryConnect()
        {
            UpdateGlobalState("Connecting...");
            try
            {
                var t = await App.BrokerSession.Reconnect();
                if (t && App.BrokerSession.IsConnected)
                {
                    IsConnectedAppBarButtonFont.Foreground = new SolidColorBrush(Windows.UI.Colors.Green);
                    IsConnectedAppBarButton.Label = ConnectedStatusLabel;
                    UpdateGlobalState(GlobalState_Connected);
                }
            }
            catch(Exception e)
            {
                UpdateGlobalState($"Failed to connect: {e.Message}");
            }
        }


        private async void TryDisconnect()
        {
            UpdateGlobalState("Disconnecting...");
            try
            {
                await App.BrokerSession.Close();
                if (! App.BrokerSession.IsConnected)
                {
                    IsConnectedAppBarButtonFont.Foreground = new SolidColorBrush(Windows.UI.Colors.Red);
                    IsConnectedAppBarButton.Label = DisconnectedStatusLabel;
                    UpdateGlobalState(GlobalState_Disconnected);
                }
            }
            catch (Exception e)
            {
                UpdateGlobalState($"Failed to disconnect: {e.Message}");
            }
        }


        private async void StartMonitoring_Click(object sender, RoutedEventArgs e)
        {
            bool success = false;
            try
            {
                success = await Task.Run(App.BrokerSession.StartMonitoring);
                if (success)
                {
                    App.DumperTask.Trigger();
                    App.DumperTask.Enabled = true;

                    StartMonitoringLabelBtn.IsEnabled = false;
                    StopMonitoringLabelBtn.IsEnabled = true;
                    App.ViewModel.IsLoading = true;

                    UpdateGlobalState(GlobalState_Capturing);
                }
            }
            catch (Exception ex)
            {
                await Utils.ShowPopUp(
                    $"Unable to start monitoring, reason:\n {ex.Message}",
                    "StartMonitoring() Error!"
                );

                if(success)
                    await Task.Run(App.BrokerSession.StopMonitoring);
            }
        }


        private async void StopMonitoring_Click(object sender, RoutedEventArgs e)
        {
            bool success = false;
            try
            {
                success = await Task.Run(App.BrokerSession.StopMonitoring);
                if ( success )
                {
                    App.DumperTask.Enabled = false;

                    StartMonitoringLabelBtn.IsEnabled = true;
                    StopMonitoringLabelBtn.IsEnabled = false;
                    App.ViewModel.IsLoading = false;

                    UpdateGlobalState(GlobalState_Connected);
                }
            }
            catch (Exception ex)
            {
                await Utils.ShowPopUp(
                    $"Unable to stop monitoring, reason:\n {ex.Message}",
                    "StopMonitoring() Error!"
                );
            }
        }


        private void ClearGridLabelBtn_Click(object sender, RoutedEventArgs e)
        {
            App.Irps.Clear();
            App.ViewModel.Irps.Clear();
        }
    }
}
