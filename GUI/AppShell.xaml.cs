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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace GUI
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class AppShell : Page
    {
        private Model.MonitoredIrps monitoredIrps;

        public readonly string MonitoredIrpsListLabel = "Monitored IRPs";
        public readonly string AboutLabel = "About CFB";
        public readonly string ReportBugUrl = "https://github.com/hugsy/cfb/issues";
        public readonly string StartMonitoringLabel = "Start Monitoring IRPs";
        public readonly string StopMonitoringLabel = "Stop Monitoring IRPs";
        public readonly string ClearGridLabel = "Clear all intercepted IRPs";


        public Frame AppFrame => frame;

        public AppShell()
        {
            this.InitializeComponent();
            monitoredIrps = new Model.MonitoredIrps();
        }

        //private void NavigateToHome_Click(object sender, RoutedEventArgs e)
        //{
            //SideMenuSv.IsPaneOpen = !SideMenuSv.IsPaneOpen;
            //this.Frame.Navigate(typeof(MainPage));
            //this.Frame.GoBack();
            //this.Frame.Navigate(typeof(Settings));
        //}


        /// <summary>
        /// The AppShell NavigationView "on tap" dispatcher
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void NavView_OnItemTapped(NavigationView sender, NavigationViewItemInvokedEventArgs args)
        {
            var label = args.InvokedItem as string;

            // default page should be the data grid
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

                //
                // TODO: Add other pages
                //
            }

            NavView.IsPaneOpen = false;
            AppFrame.Navigate(targetPage);
        }

        /// <summary>
        /// When navigating to a new page in the root frame, keep track of the 
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

                else if (e.SourcePageType == typeof(Views.SettingsPage))
                    NavView.SelectedItem = AboutMenuItem;

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
            await Launcher.LaunchUriAsync(new Uri(ReportBugUrl));


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



    }
}
