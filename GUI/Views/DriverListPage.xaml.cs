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


using GUI.ViewModels;
using GUI.Models;

namespace GUI.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class DriverListPage : Page
    {
        public DriverListPage()
        {
            this.InitializeComponent();
        }


        public DriverListPageViewModel ViewModel = new DriverListPageViewModel();


        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            ViewModel.LoadDrivers();
        }

        private void CommandBarDriverInfoButton_Click(object sender, RoutedEventArgs e)
        {
        }

        private void DataGrid_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
        {
            //todo:
            //Frame.Navigate(typeof(DriverDetailPage), ViewModel.SelectedDriver.DriverName);
        }
        
    

        // Navigates to the details page for the selected customer when the user presses SPACE.
        private void DataGrid_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Space)
            {
                //todo:
                //Frame.Navigate(typeof(DriverDetailPage), ViewModel.SelectedDriver.DriverName);
            }
        }

        private void DriverSearchBox_Loaded(object sender, RoutedEventArgs e)
        {
            if (sender is UserControls.CollapsibleSearchBox searchBox)
            {
                searchBox.AutoSuggestBox.TextChanged += DriverSearch_TextChanged;
                searchBox.AutoSuggestBox.PlaceholderText = "Filter drivers by name...";
                searchBox.AutoSuggestBox.ItemTemplate = (DataTemplate)Resources["SearchSuggestionItemTemplate"];
                searchBox.AutoSuggestBox.ItemContainerStyle = (Style)Resources["SearchSuggestionItemStyle"];
            }
        }

        private void DriverSearch_TextChanged(AutoSuggestBox sender, AutoSuggestBoxTextChangedEventArgs args)
        {
            if (args.Reason == AutoSuggestionBoxTextChangeReason.UserInput)
            {
                ViewModel.UpdateDriverSuggestions(sender.Text);
            }
        }

        private void MenuFlyoutViewDetails_Click(object sender, RoutedEventArgs e) 
        {
            //todo:
            //Frame.Navigate(typeof(DriverDetailPage), ViewModel.SelectedDriver.DriverName, new DrillInNavigationTransitionInfo());
        }

        private void SelectedDriverEnableDisableBtn_Click(object sender, RoutedEventArgs e)
        {
            var Driver = ViewModel.SelectedDriver;
            Driver.IsHooked = true;
        }
    }
}
