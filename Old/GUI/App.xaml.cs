using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using System.Collections.ObjectModel;
using Windows.ApplicationModel.Background;

using GUI.ViewModels;
using GUI.Models;
using GUI.Repositories;
using GUI.Views;
using Windows.Storage;
using Windows.UI.Core;
using System.Diagnostics;

namespace GUI
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            this.InitializeComponent();
            this.Suspending += OnSuspending;
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            
            AppShell shell = Window.Current.Content as AppShell ?? new AppShell();
            Window.Current.Content = shell;

            if (shell.AppFrame.Content == null)
            {
                Type defaultPage;

                try
                {
                    var localSettings = ApplicationData.Current.LocalSettings;
                    var homePageIndex = localSettings.Values["HomePage"] != null ? (int)localSettings.Values["HomePage"] : 0;
                    var defaultPageName = App.HomePageList[homePageIndex];
                    defaultPage = Type.GetType($"GUI.Views.{defaultPageName}");
                }
                catch (Exception)
                {
                    defaultPage = Type.GetType($"GUI.Views.{nameof(MonitoredIrpsPage)}");
                }

                // On launch the app frame content will be empty,
                // so redirect to the default page (monitored irps)
                shell.AppFrame.Navigate(
                    defaultPage,
                    null,
                    new Windows.UI.Xaml.Media.Animation.SuppressNavigationTransitionInfo()
                );
            }

            Window.Current.Activate();

            Window.Current.CoreWindow.PointerPressed += CoreWindow_PointerPressed;
        }

        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            //TODO: Save application state and stop any background activity
            deferral.Complete();
        }




        /// <summary>
        /// Static reference to the IRP list view model (main model)
        /// </summary>
        public static MonitoredIrpsViewModel ViewModel { get; } = new MonitoredIrpsViewModel();

        public static IAsyncDriverRepository Drivers { get; private set; } = new DriverRepository();

        public static IAsyncIrpRepository Irps { get; private set; } = new IrpRepository();

        public static ConnectionManager BrokerSession { get; private set; } = new ConnectionManager();

        public static IrpDumper DumperTask = new IrpDumper();


        public static readonly ObservableCollection<string> HomePageList = new ObservableCollection<string>()
        {
            nameof(MonitoredIrpsPage),
            nameof(SessionInfoPage),
            nameof(DriverListPage),
            nameof(SettingsPage),
            nameof(AboutPage)
        };


        /// <summary>
        /// Used for the In-Process background task
        /// https://docs.microsoft.com/en-us/windows/uwp/launch-resume/create-and-register-an-inproc-background-task
        /// </summary>
        /// <param name="args"></param>
        protected override void OnBackgroundActivated(BackgroundActivatedEventArgs args)
        {
            base.OnBackgroundActivated(args);
            IBackgroundTaskInstance taskInstance = args.TaskInstance;
            DumperTask.SetInstance(taskInstance);
        }


        /// <summary>
        /// Assign Back-Navigation mouse key to go back
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void CoreWindow_PointerPressed(CoreWindow sender, PointerEventArgs args)
        {
            if (args.CurrentPoint.Properties.IsXButton1Pressed)
            {
                var AppShell = Window.Current.Content as AppShell;
                if (AppShell == null)
                {
                    Debug.WriteLine("AppShell is null - should never be here");
                    return;
                }

                var AppFrame = AppShell.AppFrame; 
                if (AppFrame == null)
                {
                    Debug.WriteLine("AppFrame is null");
                    return;
                }
                
                if (AppFrame.CanGoBack)
                    AppFrame.GoBack();
            }
        }
    }
}
