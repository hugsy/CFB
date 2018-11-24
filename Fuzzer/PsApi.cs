using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Fuzzer
{
    class PsApi
    {
        [DllImport("psapi")]
        public static extern bool EnumDeviceDrivers(
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U4)] [In][Out] UIntPtr[] ddAddresses,
            UInt32 arraySizeBytes,
            [MarshalAs(UnmanagedType.U4)] out UInt32 bytesNeeded
        );

        [DllImport("psapi")]
        public static extern int GetDeviceDriverBaseName(
            UIntPtr ddAddress,
            StringBuilder ddBaseName,
            int baseNameStringSizeChars
        );

        [DllImport("psapi")]
        public static extern int GetDeviceDriverFileName(
            UIntPtr ddAddress,
            StringBuilder ddBaseName,
            int baseNameStringSizeChars
        );
    }
}
