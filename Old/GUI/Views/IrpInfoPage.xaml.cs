using GUI.Helpers;
using GUI.Models;
using GUI.ViewModels;
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

    public sealed partial class IrpInfoPage : Page
    {
        public IrpInfoPage()
        {
            this.InitializeComponent();
        }

        public IrpViewModel ViewModel = null;


        public string IrpDetailPageTitle
        {
            get
            {
                var msg = $"IRP {ViewModel.Type} sent to {ViewModel.DeviceName}";
                if (ViewModel.Model.header.Type != (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL)
                    msg += $" (IOCTL {ViewModel.IoctlCodeString})";
                return msg;
            }
        }


        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            var irp = (IrpViewModel)e.Parameter;
            if (irp == null)
            {
                await Utils.ShowPopUp("No IRP passed to the page");
                Frame.GoBack();
            }
            else
            {
                ViewModel = irp;
            }
        }


        private async void SaveAsPowerShell_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                await ViewModel.ExportAs("Powershell");
            }
            catch(Exception ex)
            {
                await Utils.ShowPopUp(ex.Message);
            }
        }

        private void ReplayIrp_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(ReplayIrpPage), ViewModel);
        }
    }
}
