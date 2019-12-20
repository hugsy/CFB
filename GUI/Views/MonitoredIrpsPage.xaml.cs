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

        }

        private void SaveAsPowershellScript_Click(object sender, RoutedEventArgs e)
        {

        }

        private void SaveAsRawFile_Click(object sender, RoutedEventArgs e)
        {

        }

        private void IrpSearchBox_Loaded(object sender, RoutedEventArgs e)
        {
            //IrpSearchBox.AutoSuggestBox.PlaceholderText = "Search IRP by RegExp...";
        }

        private void DataGrid_Sorting(object sender, DataGridColumnEventArgs e) =>
            (sender as DataGrid).Sort(e.Column, ViewModel.Irps.Sort);
    }
}
