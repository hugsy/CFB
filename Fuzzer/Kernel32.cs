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
        public const uint GENERIC_READ = 0x80000000;
        public const uint GENERIC_WRITE = 0x40000000;

        public const uint OPEN_EXISTING = 0x03;


        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateFile(
                                                     [MarshalAs(UnmanagedType.LPTStr)] string filename,
                                                     [MarshalAs(UnmanagedType.U4)] uint access,
                                                     [MarshalAs(UnmanagedType.U4)] uint share,
                                                     IntPtr securityAttributes, // optional SECURITY_ATTRIBUTES struct or IntPtr.Zero
                                                     [MarshalAs(UnmanagedType.U4)] uint creationDisposition,
                                                     [MarshalAs(UnmanagedType.U4)] uint flagsAndAttributes,
                                                     IntPtr templateFile);


        [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DeviceIoControl(  IntPtr hDevice,
                                                    uint dwIoControlCode,
                                                    IntPtr InBuffer,
                                                    uint nInBufferSize,
                                                    IntPtr OutBuffer,
                                                    uint nOutBufferSize,
                                                    IntPtr pdwBytesReturned,
                                                    IntPtr lpOverlapped);


        [DllImport("kernel32.dll", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        [SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool CloseHandle(IntPtr hObject);

        internal static void DeviceIoControl(IntPtr hDriver, uint ioctlCode, byte[] body, uint bufferLength, IntPtr zero1, int v, IntPtr pdwBytesReturned, IntPtr zero2)
        {
            throw new NotImplementedException();
        }
    }

}