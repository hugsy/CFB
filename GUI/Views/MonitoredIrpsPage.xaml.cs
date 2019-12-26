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
using Windows.ApplicationModel;
using GUI.Helpers;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.Security.Cryptography;
using Windows.Storage.Provider;
using System.Threading.Tasks;
using GUI.Models;

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

        private async void SaveAsPythonScript_Click(object sender, RoutedEventArgs e)
        {
            await CreateGenericScript("Python");
        }


        private async void SaveAsPowershellScript_Click(object sender, RoutedEventArgs e)
        {
            await CreateGenericScript("Powershell");
        }


        private async void SaveAsCScript_Click(object sender, RoutedEventArgs e)
        {
            await CreateGenericScript("C");
        }


        private async void SaveAsRawFile_Click(object sender, RoutedEventArgs e)
        {
            await CreateGenericScript("Raw");
        }


        private async Task CreateGenericScript(string _type)
        {
            if (ViewModel.SelectedIrp == null)
            {
                await Utils.ShowPopUp("No IRP selected", "Missing IRP");
                return;
            }

            if (ViewModel.SelectedIrp.Model.header.Type != (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL)
            {
                await Utils.ShowPopUp("Only IRP_MJ_DEVICE_CONTROL IRP can be replayed...", "Invalid IRP");
                return;
            }

            List<string> ValidTypes = new List<string>() {
                "Raw",
                "Powershell",
                "Python",
                "C"
            };

            if (!ValidTypes.Contains(_type))
            {
                await Utils.ShowPopUp("Invalid export type provided");
                return;
            }

            string template_filepath = null;

            if (_type != "Raw")
            {
                template_filepath = Package.Current.InstalledLocation.Path + $"\\ScriptTemplates\\{_type:s}Template.txt";
                if (!File.Exists(template_filepath))
                {
                    await Utils.ShowPopUp($"SaveAs{_type}Script(): missing template");
                    return;
                }
            }
            
            await GenerateBodyScript(_type, ViewModel.SelectedIrp, template_filepath);
        }


        private async Task GenerateBodyScript(string TypeStr, IrpViewModel irp, string template_file)
        {
            //
            // generate the script body
            //
            IBuffer output; // = new Windows.Storage.Streams.Buffer(0);

            if (TypeStr == "Raw")
            {
                output = CryptographicBuffer.CreateFromByteArray(ViewModel.SelectedIrp.InputBuffer);
            }
            else
            {
                var DeviceName = ViewModel.SelectedIrp.DeviceName.Replace(@"\Device", @"\\.");
                var IrpDataInStr = "";
                var IrpDataOutStr = "\"\"";

                foreach (byte c in ViewModel.SelectedIrp.InputBuffer)
                    IrpDataInStr += $"\\x{c:X2}";

                if (ViewModel.SelectedIrp.OutputBufferLength > 0)
                    IrpDataOutStr = $"b'\\x00'*{ViewModel.SelectedIrp.OutputBufferLength:d}";

                var fmt = File.ReadAllText(template_file);
                output = CryptographicBuffer.ConvertStringToBinary(
                        String.Format(fmt,
                        ViewModel.SelectedIrp.IoctlCode,
                        DeviceName,
                        ViewModel.SelectedIrp.DriverName,
                        $"\"{IrpDataInStr}\"",
                        IrpDataOutStr
                    ),
                    BinaryStringEncoding.Utf8
                );
            }


            //
            // write it to disk
            //
            var savePicker = new Windows.Storage.Pickers.FileSavePicker();
            savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
            savePicker.SuggestedFileName = $"Irp-0x{ViewModel.SelectedIrp.IoctlCode:x}-Session-{DateTime.Now.ToString("yyyyMMddTHH:mm:ssZ")}";

            switch (TypeStr)
            {
                case "Powershell":
                    savePicker.FileTypeChoices.Add("PowerShell", new List<string>() { ".ps1" });
                    break;

                case "Python":
                    savePicker.FileTypeChoices.Add("Python", new List<string>() { ".py" });
                    break;

                case "C":
                    savePicker.FileTypeChoices.Add("C", new List<string>() { ".c" });
                    break;
            }

            StorageFile file = await savePicker.PickSaveFileAsync();
            if (file != null)
            {
                CachedFileManager.DeferUpdates(file);
                await FileIO.WriteBufferAsync(file, output);
                FileUpdateStatus status = await CachedFileManager.CompleteUpdatesAsync(file);
                if (status == FileUpdateStatus.Complete)
                {
                    await Utils.ShowPopUp($"File {file.Name} was saved.");
                    return;
                }
            }

            await Utils.ShowPopUp($"Couldn't save IRP as a {TypeStr} script...");
            return;
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
