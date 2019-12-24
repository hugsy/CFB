using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Microsoft.Toolkit.Uwp.UI.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using GUI.ViewModels;
using System.Collections.ObjectModel;
using Microsoft.Toolkit.Uwp.Helpers;

namespace GUI.Views
{

    public sealed partial class MonitoredIrpsPage : Page
    {
        public MonitoredIrpsViewModel ViewModel 
            => App.ViewModel;


        public MonitoredIrpsPage()
        {
            this.InitializeComponent();
            Window.Current.SizeChanged += CurrentWindow_SizeChanged;
        }

        private void CurrentWindow_SizeChanged(object sender, WindowSizeChangedEventArgs e)
        {
        }

        private void SaveAsPythonScript_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException("SaveAsPythonScript");
        }

        private void SaveAsPowershellScript_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException("SaveAsPowershellScript");
        }

        private void SaveAsRawFile_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException("SaveAsRawFile");
        }

        private void IrpSearchBox_Loaded(object sender, RoutedEventArgs e)
        {
            if (IrpSearchBox != null)
            {
                IrpSearchBox.AutoSuggestBox.TextChanged += IrpSearchBox_TextChanged;
                IrpSearchBox.AutoSuggestBox.PlaceholderText = "Enter your IRP Smart Filter...";
            }
        }

        private async void IrpSearchBox_TextChanged(AutoSuggestBox sender, AutoSuggestBoxTextChangedEventArgs args)
        {
            var text = sender.Text;

            if (String.IsNullOrEmpty(text))
            {
                await ViewModel.GetIrpListAsync();
                return;
            }

            ViewModel.IsLoading = true;

             string[] parameters = text.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
             
             var matches = ViewModel.Irps
                     .Where(
                         irp => parameters.Any(
                             parameter =>
                                 irp.DeviceName.Contains(parameter, StringComparison.OrdinalIgnoreCase) ||
                                 irp.DriverName.Contains(parameter, StringComparison.OrdinalIgnoreCase) ||
                                 irp.ProcessName.Contains(parameter, StringComparison.OrdinalIgnoreCase)
                         )
                     ).ToList();
             
             
             await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
             {
                 if (matches.Count() > 0)
                 {
                     ViewModel.Irps.Clear();
             
                     foreach (var match in matches)
                         ViewModel.Irps.Add(match);
                 }
             });

            ViewModel.IsLoading = false;
        }

        private void RefreshDataGrid()
        {
            IrpDataGrid.ItemsSource = null;
            IrpDataGrid.ItemsSource = ViewModel.Irps;
        }

        private void DataGrid_Sorting(object sender, DataGridColumnEventArgs e) =>
            (sender as DataGrid).Sort(e.Column, ViewModel.Irps.Sort);

        private void ShowDetails_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(Views.IrpInfoPage), ViewModel.SelectedIrp);
        }

        private void SendToRepeater_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(Views.ReplayIrpPage), ViewModel.SelectedIrp);
        }
    }
}
