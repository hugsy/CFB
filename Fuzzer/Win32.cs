using Microsoft.Win32.SafeHandles;
using System;
using System.Drawing;
using System.Runtime.InteropServices;


/// <summary>
/// Stuff shamelessly copy-pasted from Pinvoke.net
/// </summary>
namespace Fuzzer
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


    public class Win32Keyboard
    {
        public enum VirtualKeys
        {
            LSHIFT          = 0xA0,
            RSHIFT          = 0xA1,
            LCONTROL        = 0xA2,
            RCONTROL        = 0xA3,
            LMENU           = 0xA4,
            RMENU           = 0xA5,
            F1              = 0x70,
            F2              = 0x71,
            F3              = 0x72,
            F4              = 0x73,
            F5              = 0x74,
            F6              = 0x75,
            F7              = 0x76,
            F8              = 0x77,
            F9              = 0x78,
            F10             = 0x79,
            F11             = 0x7A,
            F12             = 0x7B,
            KEY_F           = 'F',

        }


        [DllImport("user32.dll", CharSet=CharSet.Auto, ExactSpelling=true, CallingConvention=CallingConvention.Winapi)]
        public static extern ushort GetKeyState(int keyCode);

        public static bool ShiftPressed
        {
            get
            {
                ushort sStateL    = 0;
                ushort sStateR    = 0;

                sStateL = GetKeyState((int)VirtualKeys.LSHIFT);
                sStateR = GetKeyState((int)VirtualKeys.RSHIFT);

                return ((0 != (sStateL & 0x8000)) || (0 != (sStateR & 0x8000)));
            }
        }
        public static bool CtrlPressed
        {
            get
            {
                ushort sStateL    = 0;
                ushort sStateR    = 0;

                sStateL = GetKeyState((int)VirtualKeys.LCONTROL);
                sStateR = GetKeyState((int)VirtualKeys.RCONTROL);

                return ((0 != (sStateL & 0x8000)) || (0 != (sStateR & 0x8000)));
            }
        }

    }

     public class Win32Window
     {
        public enum Msgs
        {
            WM_KEYDOWN  = 0x100,
        }

        [DllImport("user32")]
        public static extern IntPtr SendMessage(HandleRef hWnd, int msg, int wParam, int lParam);

        [DllImport("user32")]
        public static extern int GetCaretPos(ref Point lpPoint);
     }
}