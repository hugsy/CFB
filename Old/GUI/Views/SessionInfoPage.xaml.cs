using GUI.ViewModels;
using GUI.Helpers;

using Microsoft.Toolkit.Uwp.Helpers;
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
using System.Threading.Tasks;

namespace GUI.Views
{

    public sealed partial class SessionInfoPage : Page
    {

        public SessionInfoViewModel ViewModel = new SessionInfoViewModel();

        public SessionInfoPage()
        {
            this.InitializeComponent();
        }


        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            ViewModel.IsLoading = true;
            ViewModel.StartPeriodicTimer();
            ViewModel.IsLoading = false;
        }


        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            ViewModel.StopPeriodicTimer();
        }
    }
}
