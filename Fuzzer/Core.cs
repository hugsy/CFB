using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace Fuzzer
{
    /// <summary>
    /// Abstraction class for interacting with the low-level functions from Core.dll
    /// </summary>
    class Core
    {

        private static string CFB_DEVICE_NAME = "\\\\.\\IrpDumper"; 
        private static string CFB_DRIVER_NAME = "IrpDumper.sys";
        private static string CFB_SERVICE_NAME = "IrpDumper";
        private static string CFB_SERVICE_DESCRIPTION = "CFB IRP Dumper Driver";

        private static IntPtr hDriver;
        private static IntPtr hSCManager;
        private static IntPtr hService;

        private static UInt32 IOCTL_AddDriver = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x801, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_RemoveDriver = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x802, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_GetNumberOfDrivers = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x803, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_ListAllDrivers = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x804, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_GetDriverInfo = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x805, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_SetEventPointer = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x806, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_EnableMonitoring = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x807, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        private static UInt32 IOCTL_DisableMonitoring = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x808, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);


        private static void PrintError(string v)
        {
            var text = $"{v}: {Kernel32.GetLastError()}";
            MessageBox.Show(text);
        }


        // from main.c
        //[DllImport(@"Core.dll")]
        //public static extern bool CheckWindowsVersion();

        public static bool CheckWindowsVersion()
        {
            return Environment.OSVersion.Version.Major >= 6;
        }

        //[DllImport(@"Core.dll")]
        //public static extern bool RunInitializationChecks();
        public static bool RunInitializationChecks()
        {
            // todo
            return true;
        }


        //[DllImport("Core.dll", SetLastError = true)]
        //public static extern bool LoadDriver();
        public static bool LoadDriver()
        {
            bool res = true;

            //
            // open the service manager and create a new service
            //
            hSCManager = WinSvc.OpenSCManager("", WinSvc.SERVICES_ACTIVE_DATABASE, WinSvc.SC_MANAGER_CREATE_SERVICE);

            if (hSCManager == IntPtr.Zero)
            {
                PrintError("OpenSCManager()");
                return false;
            }

            do
            {
                try
                {
                    string lpPath = $"{Directory.GetCurrentDirectory()}\\{CFB_DRIVER_NAME}";

                    hService = WinSvc.CreateService(
                        hSCManager,
                        CFB_SERVICE_NAME,
                        CFB_SERVICE_DESCRIPTION,
                        WinSvc.SERVICE_START | WinSvc.SERVICE_STOP | WinNt.DELETE,
                        WinNt.SERVICE_KERNEL_DRIVER,
                        WinNt.SERVICE_DEMAND_START,
                        WinNt.SERVICE_ERROR_IGNORE,
                        lpPath,
                        "", IntPtr.Zero, "", "", ""
                    );

                    //
                    // if the service was already registered, just open it
                    //
                    if (hService == IntPtr.Zero)
                    {
                    
                        if (Kernel32.GetLastError() != WinNt.ERROR_SERVICE_EXISTS)
                        {
                            PrintError("CreateService()");
                            res = false;
                            break;
                        }

                        hService = WinSvc.OpenService(
                            hSCManager,
                            CFB_SERVICE_NAME,
                            WinSvc.SERVICE_START | WinSvc.SERVICE_STOP | WinNt.DELETE
                        );

                        if (hService == IntPtr.Zero)
                        {
                            PrintError("OpenService()");
                            res = false;
                            break;
                        }
                    }


                    //
                    // start the service
                    //
                    try
                    {
                        if (WinSvc.StartService(hService, 0, IntPtr.Zero) == false)
                        {
                            PrintError("StartService()");
                            res = false;
                            break;
                        }
                    }
                    finally
                    {
                        if (!res)
                        {
                            WinSvc.CloseServiceHandle(hService);
                        }
                    }

                    res = true;
                }
                finally
                {
                    if(!res)
                    {
                        WinSvc.CloseServiceHandle(hSCManager);
                    }
                    
                }

            }
            while (false);

       
            return res;
        }




        //[DllImport(@"Core.dll")]
        //public static extern bool InitializeCfbContext();
        public static bool InitializeCfbContext()
        {

            hDriver = Kernel32.CreateFile(
                CFB_DEVICE_NAME,
                Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                0,
                IntPtr.Zero,
                Kernel32.OPEN_EXISTING,
                0,
                IntPtr.Zero
                );

            if (hDriver.ToInt32() == Kernel32.INVALID_HANDLE_VALUE)
            {
                //var text = $"Cannot open device '{DeviceName}': {Kernel32.GetLastError().ToString("x8")}";
                //MessageBox.Show(text);
                return false;
            }

            return true;
        }
        

        //[DllImport(@"Core.dll")]
        //public static extern void CleanupCfbContext();
        public static void CleanupCfbContext()
        {
            Kernel32.CloseHandle(hDriver);
        }


        //[DllImport(@"Core.dll", SetLastError = true)]
        //public static extern bool UnloadDriver();
        public static bool UnloadDriver()
        {
            bool res;

            do
            {
                if (WinSvc.ControlService(hService, WinSvc.SERVICE_CONTROL_STOP, IntPtr.Zero) == false)
                {
                    PrintError("ControlService");
                    res = false;
                    break;
                }

                if (WinSvc.DeleteService(hService) == false)
                {
                    PrintError("DeleteService");
                    res = false;
                    break;
                }

  
                WinSvc.CloseServiceHandle(hService);

                WinSvc.CloseServiceHandle(hSCManager);

                res = true;
            }
            while (false);

            return res;
        }



        private static bool GenericDeviceIoControl(uint IoctlCode, byte[] InBuf, byte[] OutBuf, out uint dwBytesReturned)
        {
            int dwlpInBufferLen;
            IntPtr lpInBuffer;
            if (InBuf != null)
            {
                dwlpInBufferLen = OutBuf.Length;
                lpInBuffer = Marshal.AllocHGlobal(dwlpInBufferLen);
            }
            else
            {
                dwlpInBufferLen = 0;
                lpInBuffer = IntPtr.Zero;
            }

            int dwlpOutBufferLen;
            IntPtr lpOutBuffer;

            if (OutBuf != null)
            {
                dwlpOutBufferLen = OutBuf.Length;
                lpOutBuffer = Marshal.AllocHGlobal(dwlpOutBufferLen);
            }
            else
            {
                dwlpOutBufferLen = 0;
                lpOutBuffer = IntPtr.Zero;
            }
            
            Marshal.Copy(InBuf, 0, lpInBuffer, InBuf.Length);
            IntPtr pdwBytesReturned = Marshal.AllocHGlobal(sizeof(uint));

            bool res = Kernel32.DeviceIoControl(
                hDriver,
                IoctlCode,
                lpInBuffer,
                (uint)dwlpOutBufferLen,
                lpOutBuffer,
                (uint)dwlpOutBufferLen,
                pdwBytesReturned,
                IntPtr.Zero
            );

            dwBytesReturned = (uint)Marshal.PtrToStructure(pdwBytesReturned, typeof(uint));
            Marshal.FreeHGlobal(pdwBytesReturned);

            if (dwlpInBufferLen != 0)
            {
                Marshal.FreeHGlobal(lpInBuffer);
            }

            if (dwlpOutBufferLen != 0)
            {
                Marshal.Copy(lpOutBuffer, OutBuf, 0, OutBuf.Length);
                Marshal.FreeHGlobal(lpOutBuffer);
            }

            return res;
        }


        // from device.c
        //[DllImport(@"Core.dll", CharSet = CharSet.Unicode)]
        //public static extern bool HookDriver(String DriverName);
        public static bool HookDriver(String DeviceName)
        {
            bool res = GenericDeviceIoControl(
                IOCTL_AddDriver, 
                Encoding.ASCII.GetBytes(DeviceName), 
                null, 
                out uint dwBytesReturned
            );
            return res && dwBytesReturned == 0;
        }


        //[DllImport(@"Core.dll", CharSet = CharSet.Unicode)]
        //public static extern bool UnhookDriver(String DriverName);
        public static bool UnhookDriver(String DeviceName)
        {
            bool res = GenericDeviceIoControl(
                IOCTL_RemoveDriver,
                Encoding.ASCII.GetBytes(DeviceName),
                null,
                out uint dwBytesReturned
            );
            return res && dwBytesReturned == 0;
        }


        //[DllImport(@"Core.dll")]
        //public static extern int GetCfbMessageHeaderSize();
        public static int GetCfbMessageHeaderSize()
        {
            // todo 
            return 1076;
        }


        [DllImport(@"Core.dll", SetLastError = true)]
        public static extern bool ReadCfbDevice(IntPtr Buffer, int BufSize, IntPtr lpNbBytesRead);
        /*
        public static bool ReadCfbDevice(IntPtr Buffer, int BufSize, out int dwNbBytesRead)
        {
            return Kernel32.ReadFile(hDriver, (byte[])Buffer, (uint)BufSize, out dwNbBytesRead, IntPtr.Zero);
        }
        */

        //[DllImport(@"Core.dll", SetLastError = true)]
        //public static extern bool SetEventNotificationHandle(IntPtr hEvent);
        public static bool SetEventNotificationHandle(IntPtr hEvent)
        {
            byte[] InBuf = Encoding.ASCII.GetBytes(hEvent.ToInt32().ToString());
            return GenericDeviceIoControl(IOCTL_SetEventPointer, InBuf, null, out uint dwBytesReturned);
        }


        //[DllImport(@"Core.dll")]
        //public static extern bool EnableMonitoring();
        public static bool EnableMonitoring()
        {
            return GenericDeviceIoControl(IOCTL_EnableMonitoring, null, null, out uint dwBytesReturned);
        }


        //[DllImport(@"Core.dll")]
        //public static extern bool DisableMonitoring();
        public static bool DisableMonitoring()
        {
            return GenericDeviceIoControl(IOCTL_DisableMonitoring, null, null, out uint dwBytesReturned);
        }
    }
}
