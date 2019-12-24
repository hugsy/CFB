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
   


        public DateTime TimeStamp       { get => DateTime.FromFileTime((long)Model.header.TimeStamp); }
        public uint ProcessId           { get => Model.header.ProcessId; }
        public uint ThreadId            { get => Model.header.ThreadId; }
        public string IrqLevel          { get => Model.IrqlAsString(); }
        public string Type              { get => Model.TypeAsString(); }
        public uint IoctlCode           { get => Model.header.IoctlCode; }
        public uint Status              { get => Model.header.Status; }
        public uint InputBufferLength   { get => Model.header.InputBufferLength; }
        public uint OutputBufferLength  { get => Model.header.OutputBufferLength; }
        public string DriverName        { get => Model.header.DriverName; }
        public string DeviceName        { get => Model.header.DeviceName; }
        public string ProcessName       { get => Model.header.ProcessName; }
        public byte[] InputBuffer       { get => Model.body.InputBuffer; }
        public byte[] OutputBuffer      { get => Model.body.OutputBuffer; }

        public string IoctlCodeString    { get => 
                Model.header.Type == (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL || Model.header.Type == (uint)IrpMajorType.IRP_MJ_INTERNAL_DEVICE_CONTROL 
                ? $"0x{IoctlCode.ToString("x8")}"
                : "N/A"; }
        public string StatusString       { get => $"0x{Status.ToString("x8")}"; }


        private string ShowStringMax(string s, int nb)
        {
            if (s == null || s.Length == 0)
                return "";

            int len = Math.Min(s.Length, nb);

            var t = s.Substring(0, len);
            if (len == nb)
                t += "[...]";
            return t;
        }


        public string InputBufferString  { get => InputBuffer != null ? ShowStringMax(BitConverter.ToString(InputBuffer), 10 * 3): ""; }
        public string OutputBufferString { get => OutputBuffer != null ? ShowStringMax(BitConverter.ToString(OutputBuffer), 10 * 3): ""; }
    }
}
