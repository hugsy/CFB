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
                if (Frame.CanGoBack)
                    Frame.GoBack();
                else
                    Frame.Navigate(typeof(Views.MonitoredIrpsPage));
                return;
            }

            if(irp.Model.header.Type != (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL)
            {
                await Utils.ShowPopUp("Only IRP_MJ_DEVICE_CONTROL IRP can be replayed");
                if (Frame.CanGoBack)
                    Frame.GoBack();
                else
                    Frame.Navigate(typeof(Views.MonitoredIrpsPage));
                return;
            }

            ViewModel = irp;
        }


        public string ReplayIrpPageTitle
        {
            get => $"Replay IRP to {ViewModel.DeviceName} with IOCTL {ViewModel.IoctlCodeString}";
        }


        private async void SendIrp_Click(object sender, RoutedEventArgs e)
        {
            // 0. empty former result
            StatusCodeTextBox.Text = "";
            OutputBufferTextBlock.Text = "";

            // 1. check fields + sanitize
            var DeviceName = DeviceNameTextBox.Text;
            if (!IsValidDeviceName(DeviceName))
                return;

            int IoctlCode;
            if (IsInt(IoctlCodeTextBox.Text))
                IoctlCode = int.Parse(IoctlCodeTextBox.Text);
            else if (IsHex(IoctlCodeTextBox.Text))
                IoctlCode = Convert.ToInt32(IoctlCodeTextBox.Text, 16);
            else
                return;

            int InputBufferLength;
            if (IsInt(InputBufferLengthTextBox.Text))
                InputBufferLength = int.Parse(InputBufferLengthTextBox.Text);
            else
                return;

            int OutputBufferLength;
            if (IsInt(OutputBufferLengthTextBox.Text))
                OutputBufferLength = int.Parse(OutputBufferLengthTextBox.Text);
            else
                return;

            byte[] InputBuffer = Utils.StringToByteArray(
                InputBufferTextBlock.Text.Replace(" ", "")
                .Replace("\r", "")
                .Replace("\n", "")
                .Replace("\t", "")
            );


            // 2. build & send forged irp
            var irp = new IrpReplay();

            try
            {
                Tuple<uint, byte[]> ioctl = await irp.SendIrp(DeviceName, IoctlCode, InputBuffer, InputBuffer.Length, OutputBufferLength);
                StatusCodeTextBox.Text = $"0x{ioctl.Item1.ToString("x8")}";
                OutputBufferTextBlock.Text = Utils.SimpleHexdump(ioctl.Item2);
            }
            catch(Exception ex)
            {
                await Utils.ShowPopUp($"Error: the following exception was triggered: {ex.Message}");
            }
        }


        private void CancelReplay_Click(object sender, RoutedEventArgs e)
            => Frame.GoBack();


        private bool IsValidDeviceName(string DeviceName)
            => DeviceName.StartsWith("\\\\.\\");


        private bool IsHex(string text)
        {
            try
            {
                Convert.ToInt32(text, 16);
                return true;
            }
            catch
            {
                return false;
            }
        }


        private bool IsInt(string text)
        {
            try
            {
                int.Parse(text);
                return true;
            }
            catch
            {
                return false;
            }
        }


        private bool IsIntOrHex(string text)
            => IsHex(text) || IsInt(text);

        private void DeviceName_TextChanged(object sender, TextChangedEventArgs e)
        {
            DeviceNameTextBox.BorderBrush = IsValidDeviceName(DeviceNameTextBox.Text) ?
                new SolidColorBrush(Windows.UI.Colors.Green):
                new SolidColorBrush(Windows.UI.Colors.Red);
        }

        private void IoctlCode_TextChanged(object sender, TextChangedEventArgs e)
        {
            IoctlCodeTextBox.BorderBrush = IsIntOrHex(IoctlCodeTextBox.Text) ?
                new SolidColorBrush(Windows.UI.Colors.Green) :
                new SolidColorBrush(Windows.UI.Colors.Red);
        }

        private void InputBufferLength_TextChanged(object sender, TextChangedEventArgs e)
        {
            InputBufferLengthTextBox.BorderBrush = IsIntOrHex(InputBufferLengthTextBox.Text) ?
                new SolidColorBrush(Windows.UI.Colors.Green) :
                new SolidColorBrush(Windows.UI.Colors.Red);
        }

        private void OutputBufferLength_TextChanged(object sender, TextChangedEventArgs e)
        {
            OutputBufferLengthTextBox.BorderBrush = IsIntOrHex(OutputBufferLengthTextBox.Text) ?
                new SolidColorBrush(Windows.UI.Colors.Green) :
                new SolidColorBrush(Windows.UI.Colors.Red);
        }
    }
}
