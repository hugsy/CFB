using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Toolkit.Uwp.Helpers;

using GUI.Models;
using GUI.Helpers;


namespace GUI.ViewModels
{
    public class DriverListPageViewModel : BindableBase
    {

        public DriverListPageViewModel() => IsLoading = false;


        public ObservableCollection<Driver> Drivers { get; private set; } = new ObservableCollection<Driver>();


        private bool _isLoading;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }


        public async void LoadDrivers()
        {
            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                IsLoading = true;
                Drivers.Clear();
            });

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                foreach (var driverName in Nt.EnumerateDirectoryObjects("\\Driver"))
                    Drivers.Add( new Driver(driverName) );

                foreach (var driverName in Nt.EnumerateDirectoryObjects("\\FileSystem"))
                    Drivers.Add(new Driver(driverName));

                IsLoading = false;
            });
        }


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
