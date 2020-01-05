using GUI.ViewModels;
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



namespace GUI.Views
{

    public sealed partial class SessionInfoPage : Page
    {

        public SessionInfoViewModel ViewModel = new SessionInfoViewModel();

        public SessionInfoPage()
        {
            this.InitializeComponent();
            this.Loaded += OnLoaded;
        }

        private async void OnLoaded(object sender, RoutedEventArgs e)
        {
            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                ViewModel.IsLoading = true;
            });

            try
            {
                await ViewModel.RefreshValues();
            }
            catch
            { }
            finally
            {
                await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
                {
                    ViewModel.IsLoading = false;
                });
            }
        }
    }
}
