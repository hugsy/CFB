using System;
using System.IO;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading;

/// <summary>
/// Yet another shameless copy-paste from Pinvoke.net
/// Interaction class for Kernel32
/// </summary>
namespace Fuzzer
{
    public class Kernel32
    {
        public const uint GENERIC_READ    = 0x80000000;
        public const uint GENERIC_WRITE   = 0x40000000;
        
        public const uint CREATE_NEW      = 1;
        public const uint CREATE_ALWAYS   = 2;
        public const uint OPEN_EXISTING   = 3;

        public const short FILE_ATTRIBUTE_NORMAL = 0x80;
        public const short INVALID_HANDLE_VALUE = -1;


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr CreateFile(
                                                string lpFileName, 
                                                uint dwDesiredAccess,
                                                uint dwShareMode, 
                                                IntPtr lpSecurityAttributes, 
                                                uint dwCreationDisposition,
                                                uint dwFlagsAndAttributes, 
                                                IntPtr hTemplateFile
            );

        
        [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool DeviceIoControl( 
                                            IntPtr hDevice,
                                            uint dwIoControlCode,
                                            IntPtr InBuffer,
                                            uint nInBufferSize,
                                            IntPtr OutBuffer,
                                            uint nOutBufferSize,
                                            IntPtr pdwBytesReturned,
                                            IntPtr lpOverlapped
            );
            
        [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool DeviceIoControl(
            IntPtr hDevice,
            uint dwIoControlCode,
            byte[] InBuffer,
            int nInBufferSize,
            byte[] OutBuffer,
            int nOutBufferSize,
            ref uint pBytesReturned,
            int pOverlapped
        );


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);


        [DllImport("kernel32.dll")]
        public static extern uint GetLastError();


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool ReadFile(
            IntPtr hFile,
            IntPtr lpBuffer,
            int nNumberOfBytesToRead,
            IntPtr lpNumberOfBytesRead, 
            IntPtr lpOverlapped
        );

    }

}