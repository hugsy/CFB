using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Helpers
{
    #region ConstantsIoctl
    namespace Constants.Ioctl
    {
        /**
         *  from ntifs.h 
         *  #define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
         *      ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
         *  )
         */
        public enum FileDeviceType : uint
        {
            FILE_DEVICE_BEEP = 0x00000001,
            FILE_DEVICE_CD_ROM = 0x00000002,
            FILE_DEVICE_CD_ROM_FILE_SYSTEM = 0x00000003,
            FILE_DEVICE_CONTROLLER = 0x00000004,
            FILE_DEVICE_DATALINK = 0x00000005,
            FILE_DEVICE_DFS = 0x00000006,
            FILE_DEVICE_DISK = 0x00000007,
            FILE_DEVICE_DISK_FILE_SYSTEM = 0x00000008,
            FILE_DEVICE_FILE_SYSTEM = 0x00000009,
            FILE_DEVICE_INPORT_PORT = 0x0000000a,
            FILE_DEVICE_KEYBOARD = 0x0000000b,
            FILE_DEVICE_MAILSLOT = 0x0000000c,
            FILE_DEVICE_MIDI_IN = 0x0000000d,
            FILE_DEVICE_MIDI_OUT = 0x0000000e,
            FILE_DEVICE_MOUSE = 0x0000000f,
            FILE_DEVICE_MULTI_UNC_PROVIDER = 0x00000010,
            FILE_DEVICE_NAMED_PIPE = 0x00000011,
            FILE_DEVICE_NETWORK = 0x00000012,
            FILE_DEVICE_NETWORK_BROWSER = 0x00000013,
            FILE_DEVICE_NETWORK_FILE_SYSTEM = 0x00000014,
            FILE_DEVICE_NULL = 0x00000015,
            FILE_DEVICE_PARALLEL_PORT = 0x00000016,
            FILE_DEVICE_PHYSICAL_NETCARD = 0x00000017,
            FILE_DEVICE_PRINTER = 0x00000018,
            FILE_DEVICE_SCANNER = 0x00000019,
            FILE_DEVICE_SERIAL_MOUSE_PORT = 0x0000001a,
            FILE_DEVICE_SERIAL_PORT = 0x0000001b,
            FILE_DEVICE_SCREEN = 0x0000001c,
            FILE_DEVICE_SOUND = 0x0000001d,
            FILE_DEVICE_STREAMS = 0x0000001e,
            FILE_DEVICE_TAPE = 0x0000001f,
            FILE_DEVICE_TAPE_FILE_SYSTEM = 0x00000020,
            FILE_DEVICE_TRANSPORT = 0x00000021,
            FILE_DEVICE_UNKNOWN = 0x00000022,
            FILE_DEVICE_VIDEO = 0x00000023,
            FILE_DEVICE_VIRTUAL_DISK = 0x00000024,
            FILE_DEVICE_WAVE_IN = 0x00000025,
            FILE_DEVICE_WAVE_OUT = 0x00000026,
            FILE_DEVICE_8042_PORT = 0x00000027,
            FILE_DEVICE_NETWORK_REDIRECTOR = 0x00000028,
            FILE_DEVICE_BATTERY = 0x00000029,
            FILE_DEVICE_BUS_EXTENDER = 0x0000002a,
            FILE_DEVICE_MODEM = 0x0000002b,
            FILE_DEVICE_VDM = 0x0000002c,
            FILE_DEVICE_MASS_STORAGE = 0x0000002d,
            FILE_DEVICE_SMB = 0x0000002e,
            FILE_DEVICE_KS = 0x0000002f,
            FILE_DEVICE_CHANGER = 0x00000030,
            FILE_DEVICE_SMARTCARD = 0x00000031,
            FILE_DEVICE_ACPI = 0x00000032,
            FILE_DEVICE_DVD = 0x00000033,
            FILE_DEVICE_FULLSCREEN_VIDEO = 0x00000034,
            FILE_DEVICE_DFS_FILE_SYSTEM = 0x00000035,
            FILE_DEVICE_DFS_VOLUME = 0x00000036,
            FILE_DEVICE_SERENUM = 0x00000037,
            FILE_DEVICE_TERMSRV = 0x00000038,
            FILE_DEVICE_KSEC = 0x00000039,
            FILE_DEVICE_FIPS = 0x0000003A,
            FILE_DEVICE_INFINIBAND = 0x0000003B,
            FILE_DEVICE_VMBUS = 0x0000003E,
            FILE_DEVICE_CRYPT_PROVIDER = 0x0000003F,
            FILE_DEVICE_WPD = 0x00000040,
            FILE_DEVICE_BLUETOOTH = 0x00000041,
            FILE_DEVICE_MT_COMPOSITE = 0x00000042,
            FILE_DEVICE_MT_TRANSPORT = 0x00000043,
            FILE_DEVICE_BIOMETRIC = 0x00000044,
            FILE_DEVICE_PMI = 0x00000045,
            FILE_DEVICE_EHSTOR = 0x00000046,
            FILE_DEVICE_DEVAPI = 0x00000047,
            FILE_DEVICE_GPIO = 0x00000048,
            FILE_DEVICE_USBEX = 0x00000049,
            FILE_DEVICE_CONSOLE = 0x00000050,
            FILE_DEVICE_NFP = 0x00000051,
            FILE_DEVICE_SYSENV = 0x00000052,
            FILE_DEVICE_VIRTUAL_BLOCK = 0x00000053,
            FILE_DEVICE_POINT_OF_SERVICE = 0x00000054,
            FILE_DEVICE_STORAGE_REPLICATION = 0x00000055,
            FILE_DEVICE_TRUST_ENV = 0x00000056,
            FILE_DEVICE_UCM = 0x00000057,
            FILE_DEVICE_UCMTCPCI = 0x00000058,
            FILE_DEVICE_PERSISTENT_MEMORY = 0x00000059,
            FILE_DEVICE_NVDIMM = 0x0000005a,
            FILE_DEVICE_HOLOGRAPHIC = 0x0000005b,
            FILE_DEVICE_SDFXHCI = 0x0000005c,
        }

        public enum AccessType : uint 
        {
            FILE_ANY_ACCESS = 0x0000,
            FILE_READ_ACCESS = 0x0001,
            FILE_WRITE_ACCESS = 0x0002,
        }

        public enum MethodType : uint
        {
            METHOD_BUFFERED = 0x0,
            METHOD_IN_DIRECT = 0x1,
            METHOD_OUT_DIRECT = 0x2,
            METHOD_NEITHER = 0x3,
        }

        public class IoctlHelperStub
        {
            public FileDeviceType DeviceType;
            public AccessType AccessType;
            public MethodType MethodType;
            public UInt16 FunctionNumber;

            public IoctlHelperStub(uint IoctlCode)
            {
                this.DeviceType = (FileDeviceType)((IoctlCode >> 16) & 0x0000ffff);
                this.AccessType = (AccessType)((IoctlCode >> 14) & 0x00000003);
                this.MethodType = (MethodType)(IoctlCode & 0x0000003);
                this.FunctionNumber = (UInt16)((IoctlCode >> 2) & 0x0000fff);
            }

            public override string ToString()
                => $"CTL_CODE(DeviceType={this.DeviceType}, Function=0x{this.FunctionNumber:x3}, Method={this.MethodType}, Access={this.AccessType})";
        }
    }
    #endregion


}
