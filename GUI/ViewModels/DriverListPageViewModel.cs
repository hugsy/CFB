using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Toolkit.Uwp.Helpers;
using Windows.UI.Popups;
using System.ComponentModel;

using GUI.Models;
using GUI.Helpers;


namespace GUI.ViewModels
{
    public class DriverListPageViewModel : BindableBase
    {


        public DriverListPageViewModel()
        {
            IsLoading = false;
            Drivers = new ObservableCollection<Driver>();
        }


        private bool _isLoading;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }

        public ObservableCollection<Driver> Drivers { get; private set; }

        public async void LoadDrivers(bool forceRefresh=false)
        {
            // use already existing driver list
            if (Drivers.Count() > 0 && forceRefresh == false)
            {
                IsLoading = false;
                return;
            }

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                IsLoading = true;
                Drivers.Clear();
            });

            try
            {
                var drivers = await App.BrokerSession.EnumerateDrivers();
                await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
                {
                    foreach (var d in drivers) 
                        Drivers.Add(d);
                });
            }
            catch (Exception e)
            {
                var dialog = new MessageDialog("Failed to enumerate drivers, reason: " + e.Message, "Driver listing failed");
                await dialog.ShowAsync();
            }

            IsLoading = false;
        }


        public void RefreshDriverList()
            => LoadDrivers(true);
        


        private Driver _selectedDriver;

        public Driver SelectedDriver
        {
            get => _selectedDriver;
            set => Set(ref _selectedDriver, value);
        }


        public ObservableCollection<Driver> DriverSuggestions { get; } = new ObservableCollection<Driver>();

        public void UpdateDriverSuggestions(string queryText)
        {
            DriverSuggestions.Clear();
            if (!string.IsNullOrEmpty(queryText))
            {
                foreach (Driver driver in Drivers)
                {
                    if(driver.DriverName.Contains(queryText))
                        DriverSuggestions.Add(driver);
                }
            }
        }
    }
}
