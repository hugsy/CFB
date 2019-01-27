using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
using System.Windows.Forms;

namespace Fuzzer
{
    public class CoreInitializationException : Exception
    {
        public CoreInitializationException() { }

        public CoreInitializationException(string message) : base(message) { }

        public CoreInitializationException(string message, Exception inner) : base(message, inner) { }
    }


    class Core
    {
        
        private static IntPtr hDriver;
        private static IntPtr hSCManager;
        private static IntPtr hService;

        //
        // From Driver\IoctlCodes.h
        //
        public static UInt32 IOCTL_AddDriver = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x801, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_RemoveDriver = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x802, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_GetNumberOfDrivers = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x803, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_ListAllDrivers = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x804, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_GetDriverInfo = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x805, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_SetEventPointer = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x806, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_EnableMonitoring = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x807, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_DisableMonitoring = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x808, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);
        public static UInt32 IOCTL_StoreTestCase = Wdm.CTL_CODE(Wdm.FILE_DEVICE_UNKNOWN, 0x809, Wdm.METHOD_BUFFERED, Wdm.FILE_ANY_ACCESS);


        public static bool CheckWindowsVersion()
        {
            return Environment.OSVersion.Version.Major >= 6;
        }


        public static bool RunInitializationChecks()
        {
            WindowsPrincipal principal = new WindowsPrincipal(WindowsIdentity.GetCurrent());
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        
        public static bool LoadDriver()
        {
            bool res = true;

            //
            // open the service manager and create a new service
            //
            hSCManager = WinSvc.OpenSCManager(
                "", 
                WinSvc.SERVICES_ACTIVE_DATABASE, 
                WinSvc.SC_MANAGER_ALL_ACCESS
            );

            if (hSCManager == IntPtr.Zero)
            {
                throw new CoreInitializationException("OpenSCManager()");
            }

            do
            {

                string lpPath = $"{Directory.GetCurrentDirectory()}\\{Settings.CfbDriverFilename}";

                hService = WinSvc.CreateService(
                    hSCManager,
                    Settings.CfbDriverShortName,
                    Settings.CfbDriverDescription,
                    WinSvc.SERVICE_START | WinSvc.SERVICE_STOP | WinNt.DELETE,
                    WinNt.SERVICE_KERNEL_DRIVER,
                    WinNt.SERVICE_DEMAND_START,
                    WinNt.SERVICE_ERROR_IGNORE,
                    lpPath,
                    IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, String.Empty
                );

                //
                // if the service was already registered, just open it
                //
                if (hService == IntPtr.Zero)
                {
                    
                    if (Kernel32.GetLastError() != WinNt.ERROR_SERVICE_EXISTS)
                    {
                        throw new CoreInitializationException("CreateService()");
                    }

                    hService = WinSvc.OpenService(
                        hSCManager,
                        Settings.CfbDriverShortName,
                        WinSvc.SERVICE_START | WinSvc.SERVICE_STOP | WinNt.DELETE
                    );

                    if (hService == IntPtr.Zero)
                    {
                        throw new CoreInitializationException("OpenService()");
                    }
                }


                //
                // start the service
                //
                if (WinSvc.StartService(hService, 0, IntPtr.Zero) == false)
                {
                    throw new CoreInitializationException("StartService()");
                }

                res = true;

            }
            while (false);

            if(!res)
            {
                if(hService != IntPtr.Zero)
                    WinSvc.CloseServiceHandle(hService);

                if (hSCManager != IntPtr.Zero)
                    WinSvc.CloseServiceHandle(hSCManager);
            }
       
            return res;
        }


        public static bool OpenDeviceHandle()
        {

            hDriver = Kernel32.CreateFile(
                Settings.CfbDevicePath,
                Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                0,
                IntPtr.Zero,
                Kernel32.OPEN_EXISTING,
                0,
                IntPtr.Zero
                );

            if (hDriver.ToInt32() == Kernel32.INVALID_HANDLE_VALUE)
            {
                throw new CoreInitializationException($"CreateFile('{Settings.CfbDevicePath}')");
            }

            return true;
        }

        
        public static bool CloseDeviceHandle()
        {
            return Kernel32.CloseHandle(hDriver);
        }


        public static bool UnloadDriver()
        {
            bool res;

            do
            {
                WinSvc.SERVICE_STATUS ServiceStatus = new WinSvc.SERVICE_STATUS();

                if (WinSvc.ControlService(hService, WinSvc.SERVICE_CONTROL_STOP, ref ServiceStatus) == false)
                {
                    throw new CoreInitializationException("ControlService()");
                }

                if (WinSvc.DeleteService(hService) == false)
                {
                    throw new CoreInitializationException("DeleteService()");
                }

  
                WinSvc.CloseServiceHandle(hService);
                hService = IntPtr.Zero;

                WinSvc.CloseServiceHandle(hSCManager);
                hSCManager = IntPtr.Zero;

                res = true;
            }
            while (false);

            return res;
        }



        private static bool GenericDeviceIoControl(uint IoctlCode, byte[] InBuf, byte[] OutBuf, out uint dwBytesReturned)
        {
            dwBytesReturned = 0;

            bool res = Kernel32.DeviceIoControl(
                hDriver,
                IoctlCode,
                InBuf,
                InBuf.Length,
                OutBuf,
                OutBuf.Length,
                ref dwBytesReturned,
                0
            );

            return res;
        }


        public static int GetCfbMessageHeaderSize()
        {
            // todo 
            return 1076;
        }



        //
        // Low-level IOCTL command wrappers
        //

        public static bool HookDriver(String DriverName)
        {
            byte[] InBuf = Encoding.Unicode.GetBytes(DriverName + "\x00");
            bool res = GenericDeviceIoControl(IOCTL_AddDriver, InBuf, new byte[0], out uint dwBytesReturned);
            return res && dwBytesReturned == 0;
        }


        public static bool UnhookDriver(String DriverName)
        {
            byte[] InBuf = Encoding.Unicode.GetBytes(DriverName + "\x00");
            bool res = GenericDeviceIoControl(IOCTL_RemoveDriver, InBuf, new byte[0], out uint dwBytesReturned);
            return res && dwBytesReturned == 0;
        }


        public static bool ReadDevice(IntPtr Buffer, int BufSize, IntPtr dwNbBytesRead)
        {
            return Kernel32.ReadFile(hDriver, Buffer, BufSize, dwNbBytesRead, IntPtr.Zero);
        }
        

        public static bool SetEventNotificationHandle(IntPtr hEvent)
        {
            long value;
            if (Environment.Is64BitProcess)
            {
                value = hEvent.ToInt64();
            }
            else
            {
                value = hEvent.ToInt32();
            }
            byte[] InBuf = BitConverter.GetBytes(value);
            return GenericDeviceIoControl(IOCTL_SetEventPointer, InBuf, new byte[0], out uint dwBytesReturned);
        }


        public static bool EnableMonitoring()
        {
            return GenericDeviceIoControl(IOCTL_EnableMonitoring, new byte[0], new byte[0], out uint dwBytesReturned);
        }


        public static bool DisableMonitoring()
        {
            return GenericDeviceIoControl(IOCTL_DisableMonitoring, new byte[0], new byte[0], out uint dwBytesReturned);
        }


        public static bool StoreLastIrpData(byte[] Buffer)
        {
            return GenericDeviceIoControl(
                IOCTL_StoreTestCase, 
                Buffer, 
                new byte[0], 
                out uint dwBytesReturned
            );
        }
    }
}
