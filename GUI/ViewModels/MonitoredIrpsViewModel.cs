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
    class MonitoredIrpsViewModel
    {
        private MonitoredIrps _model;

        public MonitoredIrpsViewModel()
        {
            _model = new MonitoredIrps();
        }

    }
}
