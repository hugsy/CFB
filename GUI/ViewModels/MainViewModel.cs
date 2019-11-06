using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.Toolkit.Uwp.Helpers;

namespace GUI.ViewModels
{
    public class MainViewModel : BindableBase
    {

        public MainViewModel() => Task.Run(GetIrpListAsync);


        public ObservableCollection<MonitoredIrpViewModel> Irps { get; } = new ObservableCollection<MonitoredIrpViewModel>();


        private MonitoredIrpViewModel _selectedIrp;


        public MonitoredIrpViewModel SelectedIrp
        {
            get => _selectedIrp;
            set => Set(ref _selectedIrp, value);
        }

        private bool _isLoading = false;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }

        public async Task GetIrpListAsync()
        {
            await DispatcherHelper.ExecuteOnUIThreadAsync(() => {
                IsLoading = true;
            });

            var irps = await App.Irps.GetAsync();
            if (irps == null)
            {
                return;
            }

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                Irps.Clear();
                foreach (var irp in irps)
                {
                    Irps.Add(new MonitoredIrpViewModel(irp));
                }
                IsLoading = false;
            });
        }

    }


}
