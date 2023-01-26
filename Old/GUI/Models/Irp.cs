using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using Newtonsoft.Json.Linq;

namespace GUI.Models
{

    public class IrpHeader
    {
        public ulong TimeStamp=0;
        public uint IrqLevel=0;
        public uint Type=0;
        public uint IoctlCode=0;
        public uint ProcessId=0;
        public uint ThreadId=0;
        public uint InputBufferLength=0;
        public uint OutputBufferLength=0;
        public uint Status=0;
        public string DriverName="";
        public string DeviceName="";
        public string ProcessName="";
    }


    public class IrpBody
    {
        public byte[] InputBuffer;
        public byte[] OutputBuffer;
    }

    public enum IrqLevel : uint
    {
        // Software
        PASSIVE_LEVEL                       = 0,
        LOW_LEVEL                           = 0,
        APC_LEVEL                           = 1,
        DPC_LEVEL                           = 2,

        // Hardware
        // TODO: add the rest
        // DIRQL_[3-26]
        PROFILE_LEVEL                       = 27, // 0x1B = timer used for profiling.
        CLOCK1_LEVEL                        = 28, // 0x1C = Interval clock 1 level - Not used on x86
        CLOCK2_LEVEL                        = 28, // 0x1C = Interval clock 2 level
        SYNCH_LEVEL                         = 28, // 0x1C = synchronization level
        IPI_LEVEL                           = 29, // 0x1D = Interprocessor interrupt level
        POWER_LEVEL                         = 30, // 0x1E = Power failure level
        HIGH_LEVEL                          = 31, // 0x1F = Highest interrupt level
    }


    public enum IrpMajorType : uint
    {
        IRP_MJ_CREATE                       = 0x00,
        IRP_MJ_CREATE_NAMED_PIPE            = 0x01,
        IRP_MJ_CLOSE                        = 0x02,
        IRP_MJ_READ                         = 0x03,
        IRP_MJ_WRITE                        = 0x04,
        IRP_MJ_QUERY_INFORMATION            = 0x05,
        IRP_MJ_SET_INFORMATION              = 0x06,
        IRP_MJ_QUERY_EA                     = 0x07,
        IRP_MJ_SET_EA                       = 0x08,
        IRP_MJ_FLUSH_BUFFERS                = 0x09,
        IRP_MJ_QUERY_VOLUME_INFORMATION     = 0x0a,
        IRP_MJ_SET_VOLUME_INFORMATION       = 0x0b,
        IRP_MJ_DIRECTORY_CONTROL            = 0x0c,
        IRP_MJ_FILE_SYSTEM_CONTROL          = 0x0d,
        IRP_MJ_DEVICE_CONTROL               = 0x0e,
        IRP_MJ_INTERNAL_DEVICE_CONTROL      = 0x0f,
        IRP_MJ_SHUTDOWN                     = 0x10,
        IRP_MJ_LOCK_CONTROL                 = 0x11,
        IRP_MJ_CLEANUP                      = 0x12,
        IRP_MJ_CREATE_MAILSLOT              = 0x13,
        IRP_MJ_QUERY_SECURITY               = 0x14,
        IRP_MJ_SET_SECURITY                 = 0x15,
        IRP_MJ_POWER                        = 0x16,
        IRP_MJ_SYSTEM_CONTROL               = 0x17,
        IRP_MJ_DEVICE_CHANGE                = 0x18,
        IRP_MJ_QUERY_QUOTA                  = 0x19,
        IRP_MJ_SET_QUOTA                    = 0x1a,
        IRP_MJ_PNP                          = 0x1b,
        IRP_MJ_PNP_POWER                    = IRP_MJ_PNP,
        IRP_MJ_MAX                          = IRP_MJ_PNP_POWER
    };


    /// <summary>
    /// Represents IRP
    /// </summary>
    public class Irp : IEquatable<Irp>
    {
        public IrpHeader header;
        public IrpBody body;

        public Irp() 
        {
            header = new IrpHeader();
            body = new IrpBody();
        }


        public bool Equals(Irp other) =>
            header.TimeStamp    == other.header.TimeStamp &&
            header.IrqLevel     == other.header.IrqLevel &&
            header.Type         == other.header.Type &&
            header.IoctlCode    == other.header.IoctlCode &&
            header.ProcessId    == other.header.ProcessId && 
            header.ThreadId     == other.header.ThreadId &&
            header.Status       == other.header.Status &&
            header.DriverName   == other.header.DriverName && 
            header.DeviceName   == other.header.DeviceName &&
            body.InputBuffer    == other.body.InputBuffer &&
            body.OutputBuffer   == other.body.OutputBuffer;


        public override string ToString() => 
            $"IRP{{'{header.DeviceName}', IRQL: {IrqlAsString()}, Type:{TypeAsString()}, PID:#{header.ProcessId} }}";


        public static string TypeAsString(UInt32 type)
        {
            var IrpType = (IrpMajorType)type;
            return $"{Enum.GetName(typeof(IrpMajorType), IrpType)}";
        }


        public string TypeAsString()
            => $"{TypeAsString(header.Type)} (0x{header.Type:x})";


        public static string IrqlAsString(UInt32 irql)
        {
            var irqLevel = (IrqLevel)irql;
            return $"{Enum.GetName(typeof(IrqLevel), irqLevel)}";
        }


        public string IrqlAsString()
           => $"{IrqlAsString(header.IrqLevel)} - 0x{header.IrqLevel:x}";

    }
}
