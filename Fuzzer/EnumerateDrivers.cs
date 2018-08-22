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


        [StructLayout(LayoutKind.Sequential, Pack = 0)]
        public unsafe struct UNICODE_STRING
        {
            public ushort Length;
            public ushort MaximumLength;
            public char* Buffer;

        }

        [StructLayout(LayoutKind.Sequential)]
        public struct OBJECT_DIRECTORY_INFORMATION
        {
            public UNICODE_STRING Name;
            public UNICODE_STRING TypeName;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 0)]
        public unsafe struct OBJECT_ATTRIBUTES
        {
            public Int32 Length;
            public IntPtr RootDirectory;
            public void* ObjectName;
            public uint Attributes;
            public IntPtr SecurityDescriptor;
            public IntPtr SecurityQualityOfService;

        }

        [DllImport("ntdll.dll")]
        public static extern int NtOpenDirectoryObject(
           out IntPtr DirectoryHandle,
           uint DesiredAccess,
           ref OBJECT_ATTRIBUTES ObjectAttributes);

        [DllImport("ntdll.dll")]
        public static unsafe extern int NtQueryDirectoryObject(
           IntPtr DirectoryHandle,
           void* Buffer,
           int Length,
           bool ReturnSingleEntry,
           bool RestartScan,
           ref uint Context,
           out uint ReturnLength);

        public static unsafe List<String> GetAllDriverObjects()
        {
            OBJECT_ATTRIBUTES objAttributes = new OBJECT_ATTRIBUTES();
            UNICODE_STRING directoryName = new UNICODE_STRING();
            string DriverPath = "\\driver";
            var buffer = Marshal.AllocCoTaskMem(1 << 16);
            var ObjectDirectoryInformation  = (OBJECT_DIRECTORY_INFORMATION*)buffer.ToPointer();
            var drivers = new List<string>(128);
            fixed (char* sname = DriverPath)
            {
                directoryName.Buffer = sname;
                directoryName.Length = (ushort)(DriverPath.Length*2);

                objAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
                objAttributes.ObjectName = &directoryName;
                objAttributes.Attributes = 0x40;
                if (0 == NtOpenDirectoryObject(out var hDirectory, 3, ref objAttributes))
                {
                    var first = true;
                    uint index = 0;
                    var status = NtQueryDirectoryObject(hDirectory, ObjectDirectoryInformation , 1 << 16, false, first, ref index, out var returned);
                    if (status == 0)
                    {
                        for (int i = 0; i < index; i++)
                        {
                            drivers.Add(new string(ObjectDirectoryInformation[i].Name.Buffer));
                        }
                    }
                }
            }
            Marshal.FreeCoTaskMem(buffer);
            return drivers;
        }

    }
}
