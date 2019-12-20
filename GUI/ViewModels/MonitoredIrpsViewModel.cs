using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.ComponentModel;

using GUI.ViewModels;
using GUI.Models;
using Microsoft.Toolkit.Uwp.Helpers;

namespace GUI.ViewModels
{
    public class MonitoredIrpsViewModel : BindableBase
    {
        public ObservableCollection<IrpViewModel> Irps { get; } = new ObservableCollection<IrpViewModel>();

        public bool IsModified { get; set; }

        public MonitoredIrpsViewModel() 
            => Task.Run(GetIrpListAsync);


        private IrpViewModel _model;


        public IrpViewModel Model
        {
            get => _model;
            set
            {
                if (_model != value)
                {
                    _model = value;
                }
            }
        }

        private IrpViewModel _lastSelectedIrp;
        private IrpViewModel _selectedIrp;


        public IrpViewModel SelectedIrp
        {
            get
            {
                _lastSelectedIrp = _selectedIrp;
                return _selectedIrp;
            }
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
                IsLoading = false;
                return;
            }

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                Irps.Clear();
                foreach (var irp in irps)
                {
                    Irps.Add(new IrpViewModel(irp));
                }
                IsLoading = false;
            });
        }

    }
}
