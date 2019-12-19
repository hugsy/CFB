using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.ViewModels
{
    public class IrpViewModel
    {
        public DateTime TimeStamp { get; set; }
        public uint ProcessId { get; set; }
        public uint ThreadId { get; set; }
        public uint IrqLevel { get; set; }
        public string Type { get; set; }
        public uint IoctlCode { get; set; }
        public uint InputBufferLength { get; set; }
        public uint OutputBufferLength { get; set; }
        public string DriverName { get; set; }
        public string DeviceName { get; set; }
        public string ProcessName { get; set; }
    }
}
