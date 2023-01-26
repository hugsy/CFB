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
using System.Threading.Tasks;
using System.Diagnostics;


using GUI.Helpers;
using GUI.ViewModels;


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


        public SaveLoadIrpsViewModel ViewModel = new SaveLoadIrpsViewModel();


        private async void DumpIrpsToFileButton_Click(object sender, RoutedEventArgs e)
        {
            SaveIrpBtn.IsEnabled = false;
            ViewModel.IsLoading = true;

            try
            {
                var savePicker = new Windows.Storage.Pickers.FileSavePicker();
                savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
                savePicker.SuggestedFileName = $"Session-{DateTime.Now.ToString("yyyyMMddTHH:mm:ssZ")}";
                savePicker.FileTypeChoices.Add("SQLite", new List<string>() { ".cfb" });

                StorageFile file = await savePicker.PickSaveFileAsync();
                if (file != null)
                {
                    CachedFileManager.DeferUpdates(file);
                    FileUpdateStatus status = await CachedFileManager.CompleteUpdatesAsync(file);
                    if (status == FileUpdateStatus.Complete)
                    {
                        var db = await ViewModel.DumpIrpsToFile();
                        await db.MoveAndReplaceAsync(file);
                        ViewModel.Status = $"✔ IRPs saved as '{db.Path}'!";
                        return;
                    }
                }

                ViewModel.Status = "✘ The IRPs were not saved.";
            } 
            catch(Exception ex)
            {
                await Utils.ShowPopUp(
                    $"An error occured while trying to save IRPs to file.\nReason:\n{ex.Message}",
                    "Save IRPs Failed"
                );
            }
            finally
            {
                SaveIrpBtn.IsEnabled = true;
                ViewModel.IsLoading = false;
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
