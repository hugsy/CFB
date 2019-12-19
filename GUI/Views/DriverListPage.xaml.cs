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
using Microsoft.Toolkit.Uwp.UI.Controls;
using Microsoft.Toolkit.Uwp.Helpers;
using System.Threading.Tasks;
using Windows.UI.Popups;

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
        }

        private void CommandBarDriverInfoButton_Click(object sender, RoutedEventArgs e)
        {
        }

        private void DataGrid_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
        {
        }    
    

        private void DataGrid_KeyDown(object sender, KeyRoutedEventArgs e)
        {
        }


        private void DataGrid_Sorting(object sender, DataGridColumnEventArgs e)
            => (sender as DataGrid).Sort(e.Column, ViewModel.Drivers.Sort);
        
    

        private void DriverSearchBox_Loaded(object sender, RoutedEventArgs e)
        {
            if(DriverSearchBox != null)
            {
                DriverSearchBox.AutoSuggestBox.TextChanged += DriverSearchBox_TextChanged;
                DriverSearchBox.AutoSuggestBox.QuerySubmitted += DriverSearchBox_QuerySubmitted;
                DriverSearchBox.AutoSuggestBox.PlaceholderText = "Filter driver by name...";
                DriverSearchBox.AutoSuggestBox.ItemTemplate = (DataTemplate)Resources["SearchSuggestionItemTemplate"];
                DriverSearchBox.AutoSuggestBox.ItemContainerStyle = (Style)Resources["SearchSuggestionItemStyle"];
            }
        }

        private async void DriverSearchBox_QuerySubmitted(AutoSuggestBox sender, AutoSuggestBoxQuerySubmittedEventArgs args)
        {
            if (!String.IsNullOrEmpty(args.QueryText))
            {
                string[] parameters = sender.Text.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                var matches = ViewModel.Drivers
                        .Where(
                            driver => parameters.Any(
                                parameter =>
                                    driver.Name.StartsWith(parameter, StringComparison.OrdinalIgnoreCase)
                            )
                        ).OrderByDescending(
                            driver => parameters.Count(
                                parameter =>
                                    driver.Name.StartsWith(parameter, StringComparison.OrdinalIgnoreCase)
                            )
                        ).ToList();

                await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
                {
                    ViewModel.Drivers.Clear();
                    foreach (var match in matches)
                    {
                        ViewModel.Drivers.Add(match);
                    }
                });
            }
        }


        private void DriverSearchBox_TextChanged(AutoSuggestBox sender, AutoSuggestBoxTextChangedEventArgs args)
        {
            if (args.Reason == AutoSuggestionBoxTextChangeReason.UserInput)
            {
                if (!String.IsNullOrEmpty(sender.Text))
                {
                    string[] parameters = sender.Text.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                    sender.ItemsSource = ViewModel.Drivers
                        .Where(
                            driver => parameters.Any(
                                parameter =>
                                    driver.Name.Contains(parameter, StringComparison.OrdinalIgnoreCase)
                            )
                        ).OrderByDescending(
                            driver => parameters.Count(
                                parameter =>
                                    driver.Name.Contains(parameter, StringComparison.OrdinalIgnoreCase)
                            )
                        ).Select(driver => $"{driver.Name}");
                }
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
