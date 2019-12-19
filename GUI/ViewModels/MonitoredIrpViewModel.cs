using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.ComponentModel;

using GUI.ViewModels;
using GUI.Models;


namespace GUI.ViewModels
{
    public class MonitoredIrpViewModel : BindableBase
    {
        
        public bool IsModified { get; set; }

        public MonitoredIrpViewModel(IrpViewModel irp = null) 
            => Model = irp ?? new IrpViewModel();


        private IrpViewModel _model;

        /// <summary>
        /// In the view, get the associated IRP object
        /// </summary>
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

    }
}
