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

using GUI.Helpers;
using GUI.Models;
using GUI.ViewModels;


namespace GUI.Views
{
    /// <summary>
    /// Page associated to loading previously saved IRPs into the current session
    /// </summary>
    public sealed partial class LoadIrpsFromFilePage : Page
    {

        public LoadIrpsFromFilePage()
        {
            this.InitializeComponent();
        }


        public SaveLoadIrpsViewModel ViewModel = new SaveLoadIrpsViewModel();


        private async void LoadIrpDatabaseButton_Click(object sender, RoutedEventArgs e)
        {
            LoadIrpsBtn.IsEnabled = false;
            ViewModel.IsLoading = true;

            try
            {
                var openPicker = new Windows.Storage.Pickers.FileOpenPicker();
                openPicker.ViewMode = Windows.Storage.Pickers.PickerViewMode.Thumbnail;
                openPicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
                openPicker.FileTypeFilter.Add(".cfb");
                Windows.Storage.StorageFile file = await openPicker.PickSingleFileAsync();
                if (file != null)
                {
                    ViewModel.Status = $"🡆 Loading file '{file.Path}'...";
                    var buffer = await Windows.Storage.FileIO.ReadBufferAsync(file);
                    byte[] fileContent = buffer.ToArray();

                    ViewModel.Status = $"🡆 Parsing content of file '{file.Path}'...";

                    ViewModel.LoadIrpsFromFile(file);

                    ViewModel.Status = $"✔ IRPs from '{file.Path}' loaded!";
                    return;
                }

                ViewModel.Status = "✘ Couldn't load IRPs from file.";
            }
            catch (Exception ex)
            {
                await Utils.ShowPopUp(
                    $"An error occured while trying to load IRPs to file.\nReason:\n{ex.Message}",
                    "Save IRPs Failed"
                );
            }
            finally
            {
                LoadIrpsBtn.IsEnabled = true;
                ViewModel.IsLoading = false;
            }

            return;
        }

    }
}
