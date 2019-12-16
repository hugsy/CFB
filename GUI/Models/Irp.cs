using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using Newtonsoft.Json.Linq;

namespace GUI.Models
{
    
    /// <summary>
    /// Represents IRP
    /// </summary>
    public class Irp : DbObject, IEquatable<Irp>
    {
        public enum IrpMajorType : uint
        {
            IRP_MJ_CREATE = 0x00,
            IRP_MJ_CREATE_NAMED_PIPE = 0x01,
            IRP_MJ_CLOSE = 0x02,
            IRP_MJ_READ = 0x03,
            IRP_MJ_WRITE = 0x04,
            IRP_MJ_QUERY_INFORMATION = 0x05,
            IRP_MJ_SET_INFORMATION = 0x06,
            IRP_MJ_QUERY_EA = 0x07,
            IRP_MJ_SET_EA = 0x08,
            IRP_MJ_FLUSH_BUFFERS = 0x09,
            IRP_MJ_QUERY_VOLUME_INFORMATION = 0x0a,
            IRP_MJ_SET_VOLUME_INFORMATION = 0x0b,
            IRP_MJ_DIRECTORY_CONTROL = 0x0c,
            IRP_MJ_FILE_SYSTEM_CONTROL = 0x0d,
            IRP_MJ_DEVICE_CONTROL = 0x0e,
            IRP_MJ_INTERNAL_DEVICE_CONTROL = 0x0f,
            IRP_MJ_SHUTDOWN = 0x10,
            IRP_MJ_LOCK_CONTROL = 0x11,
            IRP_MJ_CLEANUP = 0x12,
            IRP_MJ_CREATE_MAILSLOT = 0x13,
            IRP_MJ_QUERY_SECURITY = 0x14,
            IRP_MJ_SET_SECURITY = 0x15,
            IRP_MJ_POWER = 0x16,
            IRP_MJ_SYSTEM_CONTROL = 0x17,
            IRP_MJ_DEVICE_CHANGE = 0x18,
            IRP_MJ_QUERY_QUOTA = 0x19,
            IRP_MJ_SET_QUOTA = 0x1a,
            IRP_MJ_PNP = 0x1b,
            IRP_MJ_PNP_POWER = IRP_MJ_PNP,
        };

        public DateTime TimeStamp;
        public UInt32 IrqLevel;
        public UInt32 Type;
        public UInt32 IoctlCode;
        public UInt32 ProcessId;
        public UInt32 ThreadId;
        public UInt32 InputBufferLength;
        public UInt32 OutputBufferLength;
        public string DriverName;
        public string DeviceName;
        public string ProcessName;
        public byte[] Body;

        public Irp()
        {

        }


        public Irp(JObject json)
        {

        }


        public bool Equals(Irp other) =>
            TimeStamp == other.TimeStamp &&
            IrqLevel == other.IrqLevel &&
            Type == other.Type &&
            IoctlCode == other.IoctlCode &&
            ProcessId == other.ProcessId && ThreadId == other.ThreadId &&
            DriverName == other.DriverName && DeviceName == other.DeviceName &&
            Body == other.Body;


        public override string ToString() => 
            $"IRP{{'{DeviceName}', Type:{TypeAsString(this.Type)}, PID:#{this.ProcessId} }}";


        public static string TypeAsString(UInt32 type)
        {
            switch ((Irp.IrpMajorType)type)
            {
                case IrpMajorType.IRP_MJ_CREATE:                    return "IRP_MJ_CREATE"; //00
                case IrpMajorType.IRP_MJ_CREATE_NAMED_PIPE:         return "IRP_MJ_CREATE_NAMED_PIPE"; //01,
                case IrpMajorType.IRP_MJ_CLOSE:                     return "IRP_MJ_CLOSE"; //02,
                case IrpMajorType.IRP_MJ_READ:                      return "IRP_MJ_READ"; //03,
                case IrpMajorType.IRP_MJ_WRITE:                     return "IRP_MJ_WRITE"; //04,
                case IrpMajorType.IRP_MJ_QUERY_INFORMATION:         return "IRP_MJ_QUERY_INFORMATION"; //05,
                case IrpMajorType.IRP_MJ_SET_INFORMATION:           return "IRP_MJ_SET_INFORMATION"; //06,
                case IrpMajorType.IRP_MJ_QUERY_EA:                  return "IRP_MJ_QUERY_EA"; //07,
                case IrpMajorType.IRP_MJ_SET_EA:                    return "IRP_MJ_SET_EA"; //08,
                case IrpMajorType.IRP_MJ_FLUSH_BUFFERS:             return "IRP_MJ_FLUSH_BUFFERS"; //09,
                case IrpMajorType.IRP_MJ_QUERY_VOLUME_INFORMATION:  return "IRP_MJ_QUERY_VOLUME_INFORMATION"; //0a,
                case IrpMajorType.IRP_MJ_SET_VOLUME_INFORMATION:    return "IRP_MJ_SET_VOLUME_INFORMATION"; //0b,
                case IrpMajorType.IRP_MJ_DIRECTORY_CONTROL:         return "IRP_MJ_DIRECTORY_CONTROL"; //0c,
                case IrpMajorType.IRP_MJ_FILE_SYSTEM_CONTROL:       return "IRP_MJ_FILE_SYSTEM_CONTROL"; //0d,
                case IrpMajorType.IRP_MJ_DEVICE_CONTROL:            return "IRP_MJ_DEVICE_CONTROL"; //0e,
                case IrpMajorType.IRP_MJ_INTERNAL_DEVICE_CONTROL:   return "IRP_MJ_INTERNAL_DEVICE_CONTROL"; //0f,
                case IrpMajorType.IRP_MJ_SHUTDOWN:                  return "IRP_MJ_SHUTDOWN"; //10,
                case IrpMajorType.IRP_MJ_LOCK_CONTROL:              return "IRP_MJ_LOCK_CONTROL"; //11,
                case IrpMajorType.IRP_MJ_CLEANUP:                   return "IRP_MJ_CLEANUP"; //12,
                case IrpMajorType.IRP_MJ_CREATE_MAILSLOT:           return "IRP_MJ_CREATE_MAILSLOT"; //13,
                case IrpMajorType.IRP_MJ_QUERY_SECURITY:            return "IRP_MJ_QUERY_SECURITY"; //14,
                case IrpMajorType.IRP_MJ_SET_SECURITY:              return "IRP_MJ_SET_SECURITY"; //15,
                case IrpMajorType.IRP_MJ_POWER:                     return "IRP_MJ_POWER"; //16,
                case IrpMajorType.IRP_MJ_SYSTEM_CONTROL:            return "IRP_MJ_SYSTEM_CONTROL"; //17,
                case IrpMajorType.IRP_MJ_DEVICE_CHANGE:             return "IRP_MJ_DEVICE_CHANGE"; //18,
                case IrpMajorType.IRP_MJ_QUERY_QUOTA:               return "IRP_MJ_QUERY_QUOTA"; //19,
                case IrpMajorType.IRP_MJ_SET_QUOTA:                 return "IRP_MJ_SET_QUOTA"; //1a,
                case IrpMajorType.IRP_MJ_PNP:                       return "IRP_MJ_PNP";//0x1b,
            }
            return $"IRP_MJ_UNKNOWN_NUM_{type}";
        }


        public string TypeAsString()
        {
            return TypeAsString(this.Type);
        }


        public static string IrqlAsString(UInt32 irql)
        {
            switch (irql)
            {
                case 0:                    return "PASSIVE_LEVEL";
                case 1:                    return "APC_LEVEL";
                case 2:                    return "DPC_LEVEL";
            }
            return "<unknown>";
        }

        public string IrqlAsString()
        {
            return IrqlAsString(this.IrqLevel);
        }

    }
}
