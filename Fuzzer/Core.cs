using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Fuzzer
{
    /// <summary>
    /// Abstraction class for interacting with the low-level functions from Core.dll
    /// </summary>
    class Core
    {

        // from main.c
        [DllImport(@"Core.dll")]
        public static extern bool CheckWindowsVersion();

        [DllImport(@"Core.dll")]
        public static extern bool RunInitializationChecks();

        [DllImport("Core.dll")]
        public static extern bool LoadDriver();

        [DllImport(@"Core.dll")]
        public static extern bool InitializeCfbContext();

        [DllImport(@"Core.dll")]
        public static extern void CleanupCfbContext();

        [DllImport(@"Core.dll")]
        public static extern bool UnloadDriver();


        // from device.c
        [DllImport(@"Core.dll", CharSet = CharSet.Unicode)]
        public static extern bool HookDriver(String DriverName);

        [DllImport(@"Core.dll", CharSet = CharSet.Unicode)]
        public static extern bool UnhookDriver(String DriverName);

        [DllImport(@"Core.dll")]
        public static extern int GetCfbMessageHeaderSize();

        [DllImport(@"Core.dll", SetLastError = true)]
        public static extern bool ReadCfbDevice(IntPtr Buffer, int BufSize, IntPtr lpNbBytesRead);

        [DllImport(@"Core.dll", SetLastError = true)]
        public static extern bool SetEventNotificationHandle(IntPtr hEvent);

    }
}
