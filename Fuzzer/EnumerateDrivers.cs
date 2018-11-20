using System;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Win32.SafeHandles;

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

        public static List<Tuple<UIntPtr, String>> EnumerateAllDeviceDrivers()
        {
            long arraySize;
            UInt32 arraySizeBytes;
            UIntPtr[] ddAddresses;
            bool success;

            List<Tuple<UIntPtr, String>> Result = new List<Tuple<UIntPtr, String>>();
                
            success = EnumDeviceDrivers(null, 0, out uint bytesNeeded);

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

        

        public static List<String> EnumerateDirectoryObjects(string RootPath)
        {
#pragma warning disable IDE0018 // Inline variable declaration
            SafeFileHandle Handle;
#pragma warning restore IDE0018 // Inline variable declaration
            List<String> Res = new List<String>();

            var ObjAttr = new Win32.OBJECT_ATTRIBUTES(RootPath, Win32.OBJ_CASE_INSENSITIVE);

            var Status = Win32.NtOpenDirectoryObject(out Handle, Win32.DIRECTORY_QUERY | Win32.DIRECTORY_TRAVERSE, ref ObjAttr);
            if (Status != 0)
            {
                return Res;
            }
                

            int RawBufferSize = 128*1024;
            IntPtr RawBuffer = Marshal.AllocHGlobal(RawBufferSize);
            int ObjDirInfoStructSize = Marshal.SizeOf(typeof(Win32.OBJECT_DIRECTORY_INFORMATION));
            uint Context = 0;

            Status = Win32.NtQueryDirectoryObject(Handle, RawBuffer, RawBufferSize, false, true, ref Context, out uint ReturnLength);
            if (Status == 0)
            {             
                for (int i = 0; i < Context; i++)
                {
                    IntPtr Addr = RawBuffer + i*ObjDirInfoStructSize;
                    var ObjectDirectoryInformation = (Win32.OBJECT_DIRECTORY_INFORMATION)Marshal.PtrToStructure(Addr, typeof(Win32.OBJECT_DIRECTORY_INFORMATION));
                    Res.Add(ObjectDirectoryInformation.Name.ToString());
                }
            }

            Marshal.FreeHGlobal(RawBuffer);
            Handle.Dispose();

            return Res;
        }


    }
}
