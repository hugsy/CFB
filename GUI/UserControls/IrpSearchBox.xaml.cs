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

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace GUI.UserControls
{
    public sealed partial class IrpSearchBox : UserControl
    {
        private double RequestedWidth = 32;


        public IrpSearchBox()
        {
            this.InitializeComponent();
        }


        private void SearchButton_Checked(object sender, RoutedEventArgs e)
        {
        }

        private void SearchBox_LostFocus(object sender, RoutedEventArgs e)
        {
        }
    }
}
