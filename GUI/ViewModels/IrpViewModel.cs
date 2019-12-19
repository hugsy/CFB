using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GUI.Models;


namespace GUI.ViewModels
{
    public class IrpViewModel : BindableBase
    {

        public IrpViewModel(Irp irp = null)
            => _model = irp ?? new Irp();


        private Irp _model;
        
        public Irp Model
        {
            get => _model;
        }


        public DateTime TimeStamp       { get => Model.header.TimeStamp; }
        public uint ProcessId           { get => Model.header.ProcessId; }
        public uint ThreadId            { get => Model.header.ThreadId; }
        public uint IrqLevel            { get => Model.header.IrqLevel; }
        public string Type              { get => Model.TypeAsString(); }
        public uint IoctlCode           { get => Model.header.IoctlCode; }
        public uint InputBufferLength   { get => Model.header.InputBufferLength; }
        public uint OutputBufferLength  { get => Model.header.OutputBufferLength; }
        public string DriverName        { get => Model.header.DriverName; }
        public string DeviceName        { get => Model.header.DeviceName; }
        public string ProcessName       { get => Model.header.ProcessName; }
        public byte[] InputBuffer       { get => Model.body.InputBuffer; }
        public byte[] OutputBuffer      { get => Model.body.OutputBuffer; }
    }
}
