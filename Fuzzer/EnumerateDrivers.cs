using System;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Fuzzer
{
    class EnumerateDrivers
    {
        [DllImport("psapi")]
        private static extern bool EnumDeviceDrivers(
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U4)] [In][Out] UIntPtr[] ddAddresses,
            UInt32 arraySizeBytes,
            [MarshalAs(UnmanagedType.U4)] out UInt32 bytesNeeded
        );

        [DllImport("psapi")]
        private static extern int GetDeviceDriverBaseName(
            UIntPtr ddAddress,
            StringBuilder ddBaseName,
            int baseNameStringSizeChars
        );

        [DllImport("psapi")]
        private static extern int GetDeviceDriverFileName(
            UIntPtr ddAddress,
            StringBuilder ddBaseName,
            int baseNameStringSizeChars
        );

        public static List<Tuple<UIntPtr, String>> GetAllDrivers()
        {
            long arraySize;
            UInt32 arraySizeBytes;
            UIntPtr[] ddAddresses;
            UInt32 bytesNeeded;
            bool success;

            List<Tuple<UIntPtr, String>> Result = new List<Tuple<UIntPtr, String>>();
                
            success = EnumDeviceDrivers(null, 0, out bytesNeeded);

            if (!success || bytesNeeded == 0)
            {
                return Result;
            }

            arraySize = bytesNeeded / UIntPtr.Size;
            arraySizeBytes = bytesNeeded;
            ddAddresses = new UIntPtr[arraySize];
            success = EnumDeviceDrivers(ddAddresses, arraySizeBytes, out bytesNeeded);

            if (!success)
            {
                return Result;
            }

            for (int i = 0; i < arraySize; i++)
            {
                StringBuilder sb = new StringBuilder(1000);
                GetDeviceDriverBaseName(ddAddresses[i], sb, sb.Capacity);
                Tuple<UIntPtr, String> Entry = new Tuple<UIntPtr, string>(ddAddresses[i], sb.ToString());
                Result.Add(Entry);
            }

            return Result;
        }
    }
}
