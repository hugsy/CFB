using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Fuzzer
{
    class WinSvc
    {
        public static string SERVICES_ACTIVE_DATABASE = "ServicesActive";
        public static string SERVICES_FAILED_DATABASE = "ServicesFailed";

        public static uint SC_MANAGER_CONNECT = 0x0001;
        public static uint SC_MANAGER_CREATE_SERVICE = 0x0002;
        public static uint SC_MANAGER_ENUMERATE_SERVICE = 0x0004;
        public static uint SC_MANAGER_LOCK = 0x0008;
        public static uint SC_MANAGER_QUERY_LOCK_STATUS = 0x0010;
        public static uint SC_MANAGER_MODIFY_BOOT_CONFIG = 0x0020;

        public static uint SC_MANAGER_ALL_ACCESS = (
            WinNt.STANDARD_RIGHTS_REQUIRED |
            SC_MANAGER_CONNECT |
            SC_MANAGER_CREATE_SERVICE |
            SC_MANAGER_ENUMERATE_SERVICE |
            SC_MANAGER_LOCK |
            SC_MANAGER_QUERY_LOCK_STATUS |
            SC_MANAGER_MODIFY_BOOT_CONFIG);


        public static uint SERVICE_START = 0x0010;
        public static uint SERVICE_STOP = 0x0020;

        public static uint SERVICE_CONTROL_STOP = 0x00000001;

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SERVICE_STATUS
        {
            UInt32 dwServiceType;
            UInt32 dwCurrentState;
            UInt32 dwControlsAccepted;
            UInt32 dwWin32ExitCode;
            UInt32 dwServiceSpecificExitCode;
            UInt32 dwCheckPoint;
            UInt32 dwWaitHint;
        };
        


        [DllImport("Advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr OpenSCManager(
            string lpMachineName,
            string lpDatabaseName,
            uint dwDesiredAccess
        );


        [DllImport("Advapi32.dll", SetLastError = true, CharSet = CharSet.Auto )]
        public static extern IntPtr CreateService(
            IntPtr hSCManager,
            string lpServiceName,
            string lpDisplayName,
            uint dwDesiredAccess,
            uint dwServiceType,
            uint dwStartType,
            uint dwErrorControl,
            string lpBinaryPathName,
            IntPtr lpLoadOrderGroup,
            IntPtr lpdwTagId,
            IntPtr lpDependencies,
            IntPtr lpServiceStartName,
            string lpPassword
        );


        [DllImport("Advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr OpenService(
            IntPtr hSCManager,
            string lpServiceName,
            uint dwDesiredAccess
        );


        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern bool StartService(
          IntPtr hService,
          uint dwNumServiceArgs,
          IntPtr lpServiceArgVectors
        );


        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern bool ControlService(
          IntPtr hService,
          uint dwControl,
          ref WinSvc.SERVICE_STATUS lpServiceStatus
        );


        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern bool DeleteService(
            IntPtr hService
        );


        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern bool CloseServiceHandle(
            IntPtr hService
        );

    }
}
