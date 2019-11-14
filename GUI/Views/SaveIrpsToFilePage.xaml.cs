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
using Windows.Storage.Streams;
using Windows.Storage;
using Windows.Storage.Provider;
using Windows.Security.Cryptography;

namespace GUI.Views
{
    /// <summary>
    /// Save all the IRP objects to a serialized format to disk
    /// </summary>
    public sealed partial class SaveIrpsToFilePage : Page
    {
        public SaveIrpsToFilePage()
        {
            this.InitializeComponent();
        }



        private async void DumpIrpsToFileButton_Click(object sender, RoutedEventArgs e)
        {
            var savePicker = new Windows.Storage.Pickers.FileSavePicker();
            savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
            savePicker.SuggestedFileName = $"Session-{DateTime.Now.ToString("yyyyMMddTHH:mm:ssZ")}";
            savePicker.FileTypeChoices.Add("CSV", new List<string>() { ".cfb" });
          
            StorageFile file = await savePicker.PickSaveFileAsync();
            if (file != null)
            {
                CachedFileManager.DeferUpdates(file);
                await FileIO.WriteBufferAsync(file, GetBufferFromString(file.Name));
                FileUpdateStatus status = await CachedFileManager.CompleteUpdatesAsync(file);
                if (status == FileUpdateStatus.Complete)
                {
                    Status = "File " + file.Name + " was saved.";
                }
                else
                {
                    Status = "File " + file.Name + " couldn't be saved.";
                }
            }
            else
            {
                Status = "Operation cancelled.";
            }
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


        static internal IBuffer GetBufferFromString(string str)
        {
            if (String.IsNullOrEmpty(str))
            {
                return new Windows.Storage.Streams.Buffer(0);
            }
            else
            {
                return CryptographicBuffer.ConvertStringToBinary(str, BinaryStringEncoding.Utf8);
            }
        }

    }
    
}
