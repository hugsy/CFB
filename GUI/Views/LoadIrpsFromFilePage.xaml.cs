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

using GUI.Models;
using Windows.System;

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


        private async void LoadIrpDatabaseButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var openPicker = new Windows.Storage.Pickers.FileOpenPicker();
                openPicker.ViewMode = Windows.Storage.Pickers.PickerViewMode.Thumbnail;
                openPicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
                openPicker.FileTypeFilter.Add(".cfb");
                Windows.Storage.StorageFile file = await openPicker.PickSingleFileAsync();
                
                Status = $"Loading file '{file.Path}'...";

                var buffer = await Windows.Storage.FileIO.ReadBufferAsync(file);
                byte[] fileContent = buffer.ToArray();

                Status = $"Parsing content of file '{file.Path}'...";

                // todo implement

            }
            catch (Exception excpt)
            {
                Status = $"An error occured: {excpt.ToString()}";
            }

            return;
        }


        private string _loading_status_update;
        public string Status
        {
            private set
            {
                _loading_status_update = value;
            }
            get
            {
                return _loading_status_update;
            }
        }
    }
}
