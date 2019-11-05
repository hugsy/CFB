using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.ComponentModel;

using GUI.Models;


namespace GUI.ViewModels
{
    class MonitoredIrpViewModel
    {
        private Irp _model;
        public bool IsModified { get; set; }

        public MonitoredIrpViewModel(Irp irp = null) => Model = irp ?? new Irp();

        /// <summary>
        /// In the view, get the associated IRP object
        /// </summary>
        public Irp Model
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

    }
}
