using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace Fuzzer
{
    /// <summary>
    /// This structure mimics the structure SNIFFED_DATA_HEADER from the driver IrpDumper (IrpDumper\PipeComm.h)
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct IrpHeader
    {
        public ulong TimeStamp;
        public UInt32 Irql;
        public UInt32 Type;
        public UInt32 IoctlCode;
        public UInt32 ProcessId;
        public UInt32 ThreadId;
        public UInt32 InputBufferLength;
        public UInt32 OutputBufferLength;
    }


    public class Irp
    {
        public enum IrpMajorType : uint
        {
            READ = 0x03,
            WRITE = 0x04,
            DEVICE_CONTROL = 0x0e
        };

        public IrpHeader Header;
        public string DriverName;
        public string DeviceName;
        public string ProcessName;
        public byte[] Body;

        private Random Rnd;

        public Irp()
        {
            this.Rnd = new Random();
        }


        protected Irp(Irp another)
        {
            Header = another.Header;
            DriverName = another.DriverName;
            DeviceName = another.DeviceName;
            ProcessName = another.ProcessName;
            Body = another.Body.ToArray();
            Rnd = new Random();
        }


        public Irp Clone()
        {
            return new Irp(this);
        }


        public override string ToString()
        {
            return $"IRP{{'{DeviceName}', Type:{TypeAsString(this.Header.Type)}, PID:#{Header.ProcessId} }}";
        }

        public static string TypeAsString(UInt32 type)
        {
            switch ((Irp.IrpMajorType)type)
            {
                case IrpMajorType.READ:
                    return "READ";

                case IrpMajorType.WRITE:
                    return "WRITE";

                case IrpMajorType.DEVICE_CONTROL:
                    return "DEVICE_CONTROL";
            }

            return "<unknown>";
        }

        public string TypeAsString()
        {
            return TypeAsString(this.Header.Type);
        }

        public static string IrqlAsString(UInt32 irql)
        {
            switch (irql)
            {
                case 0:
                    return "PASSIVE_LEVEL";

                case 1:
                    return "APC_LEVEL";

                case 2:
                    return "DPC_LEVEL";
            }

            return "<unknown>";
        }

        public string IrqlAsString()
        {
            return IrqlAsString(this.Header.Irql);
        }

        public void FuzzBody()
        {
           this.Rnd.NextBytes(this.Body);
        }
    }
}
