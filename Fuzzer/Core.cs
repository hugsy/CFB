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
        [DllImport("Core.dll")]
        public static extern bool CheckWindowsVersion();

        [DllImport("Core.dll")]
        public static extern bool RunInitializationChecks();

        [DllImport("Core.dll")]
        public static extern bool LoadDriver();

        [DllImport("Core.dll")]
        public static extern bool InitializeCfbContext();

        [DllImport("Core.dll")]
        public static extern void CleanupCfbContext();

        [DllImport("Core.dll")]
        public static extern bool UnloadDriver();

        [DllImport("Core.dll")]
        private unsafe static extern bool CfbReadMessage(void* Buffer, int BufSize, int* lpNbBytesRead);
        public unsafe static bool ReadMessage(void* Buffer, int BufSize)
        {
            return CfbReadMessage(Buffer, BufSize, null);
        }

        [DllImport("Core.dll")]
        private static extern int CfbGetMessageHeaderSize();
        public static int MessageHeaderSize()
        {
            return CfbGetMessageHeaderSize();
        }


        // from device.c
        [DllImport("Core.dll", CharSet = CharSet.Unicode)]
        public static extern bool HookDriver(String DriverName);

        [DllImport("Core.dll", CharSet = CharSet.Unicode)]
        public static extern bool UnhookDriver(String DriverName);
    }
}
