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
    public class MonitoredIrpViewModel
    {
        
        public bool IsModified { get; set; }

        public MonitoredIrpViewModel(Irp irp = null) => Model = irp ?? new Irp();

        private Irp _model;

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


        public DateTime TimeStamp { get => Model.TimeStamp; }
        public uint ProcessId { get => Model.ProcessId; }
        public uint ThreadId { get => Model.ThreadId; }
        public uint IrqLevel { get => Model.IrqLevel; }
        public string Type { get => Model.TypeAsString(); }
        public uint IoctlCode { get => Model.IoctlCode; }
        public uint InputBufferLength { get => Model.InputBufferLength; }
        public uint OutputBufferLength { get => Model.OutputBufferLength;  }
        public string DriverName { get => Model.DriverName; }
        public string DeviceName { get => Model.DeviceName; }
        public string ProcessName { get => Model.ProcessName; }
    }
}
