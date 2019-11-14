using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;


/// <summary>
/// Stuff shamelessly copy-pasted from Pinvoke.net
/// Interaction class for Win32k
/// </summary>
namespace GUI.Native
{
    public class Win32
    {
        public const int OBJ_INHERIT = 2;
        public const int OBJ_PERMANENT = 16;
        public const int OBJ_EXCLUSIVE = 32;
        public const int OBJ_CASE_INSENSITIVE = 64;
        public const int OBJ_OPENIF = 128;
        public const int OBJ_OPENLINK = 256;
        public const int OBJ_VALID_ATTRIBUTES = 498;


        [StructLayout(LayoutKind.Sequential)]
        public struct UNICODE_STRING : IDisposable
        {
            public ushort Length;
            public ushort MaximumLength;
            private IntPtr Buffer;
            public UNICODE_STRING(string s)
            {
                Length = (ushort)(s.Length * 2);
                MaximumLength = (ushort)(Length + 2);
                Buffer = Marshal.StringToHGlobalUni(s);
            }
            public void Dispose()
            {
                Marshal.FreeHGlobal(Buffer);
                Buffer = IntPtr.Zero;
            }
            public override string ToString()
            {
                return Marshal.PtrToStringUni(Buffer);
            }
        }



        [StructLayout(LayoutKind.Sequential)]
        public struct OBJECT_ATTRIBUTES : IDisposable
        {
            public int Length;
            public IntPtr RootDirectory;
            private IntPtr objectName;
            public int Attributes;
            public IntPtr SecurityDescriptor;
            public IntPtr SecurityQualityOfService;

            public OBJECT_ATTRIBUTES(string name, int attrs)
            {
                Length = 0;
                RootDirectory = IntPtr.Zero;
                objectName = IntPtr.Zero;
                Attributes = attrs;
                SecurityDescriptor = IntPtr.Zero;
                SecurityQualityOfService = IntPtr.Zero;

                Length = Marshal.SizeOf(this);
                ObjectName = new UNICODE_STRING(name);
            }

            public UNICODE_STRING ObjectName
            {
                get
                {
                    return (UNICODE_STRING)Marshal.PtrToStructure(
                     objectName, typeof(UNICODE_STRING));
                }

                set
                {
                    bool fDeleteOld = objectName != IntPtr.Zero;
                    if (!fDeleteOld)
                        objectName = Marshal.AllocHGlobal(Marshal.SizeOf(value));
                    Marshal.StructureToPtr(value, objectName, fDeleteOld);
                }
            }

            public void Dispose()
            {
                if (objectName != IntPtr.Zero)
                {
                    Marshal.DestroyStructure(objectName, typeof(UNICODE_STRING));
                    Marshal.FreeHGlobal(objectName);
                    objectName = IntPtr.Zero;
                }
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct OBJECT_DIRECTORY_INFORMATION
        {
            public UNICODE_STRING Name;
            public UNICODE_STRING TypeName;
        }


        public const int DIRECTORY_QUERY = 1;
        public const int DIRECTORY_TRAVERSE = 2;
        public const int DIRECTORY_CREATE_OBJECT = 4;
        public const int DIRECTORY_CREATE_SUBDIRECTORY = 8;
        public const int DIRECTORY_ALL_ACCESS = 16;

        [DllImport("ntdll.dll")]
        public static extern int NtOpenDirectoryObject(
           out SafeFileHandle DirectoryHandle,
           uint DesiredAccess,
           ref OBJECT_ATTRIBUTES ObjectAttributes);


        [DllImport("ntdll.dll")]
        public static extern int NtQueryDirectoryObject(
           SafeFileHandle DirectoryHandle,
           IntPtr Buffer,
           int Length,
           bool ReturnSingleEntry,
           bool RestartScan,
           ref uint Context,
           out uint ReturnLength);
    }

}