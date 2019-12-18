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
using Windows.UI.Xaml;

namespace GUI.ViewModels
{
    public class DriverListPageViewModel : BindableBase
    {


        public DriverListPageViewModel()
        {
            IsLoading = false;
            Drivers = new ObservableCollection<Driver>();
        }

        public string EnableDisableSelectedDriverText
        {
            get
            {
                if (_selectedDriver == null)
                    return "Hook/Unhook";

                return SelectedDriver.IsHooked ? $"Unhook {SelectedDriver.Name}" : $"Hook {SelectedDriver.Name}";
            }
        }


        private bool _isLoading;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }

        //
        // Collection of drivers visible from the view (set or subset of App.Drivers)
        //
        public ObservableCollection<Driver> Drivers { get; private set; }

        public async void GetDriversAsync(bool forceRefresh=false)
        {
            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                IsLoading = true;
                Drivers.Clear();
            });
           
            var drivers = await App.Drivers.GetAsync(forceRefresh);

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                foreach (var d in drivers) 
                    Drivers.Add(d);
            });

            IsLoading = false;
        }


        public void ForceGetDriversAsync()
            => GetDriversAsync(true);
        


        private Driver _selectedDriver;

        public Driver SelectedDriver
        {
            get => _selectedDriver;
            set => Set(ref _selectedDriver, value);
        }


        public ObservableCollection<Driver> DriverSuggestions { get; } = new ObservableCollection<Driver>();

    }
}
