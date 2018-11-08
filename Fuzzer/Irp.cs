using System;

namespace Fuzzer
{
    public class Irp
    {
        public enum IrpMajorType : UInt32
        {
            READ = 0x03,
            WRITE = 0x04,
            DEVICE_CONTROL = 0x0e
        };

        public CfbDataReader.CfbMessageHeader Header;
        public string DriverName;
        public string DeviceName;
        public string ProcessName;
        public byte[] Body;

        public Irp()
        {
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
    }
}
