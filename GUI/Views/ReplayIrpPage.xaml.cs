using GUI.Helpers;
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

    public sealed partial class ReplayIrpPage : Page
    {
        public ReplayIrpPage()
        {
            this.InitializeComponent();
        }


        public IrpViewModel ViewModel = null;


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
    }
}
