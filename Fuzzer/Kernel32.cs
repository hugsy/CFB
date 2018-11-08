using System;
using System.Runtime.InteropServices;

/// <summary>
/// Yet another shameless copy-paste from Pinvoke.net
/// </summary>
namespace Fuzzer
{
    public class Kernel32
    {
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateFile(
                                                     [MarshalAs(UnmanagedType.LPTStr)] string filename,
                                                     [MarshalAs(UnmanagedType.U4)] FileAccess access,
                                                     [MarshalAs(UnmanagedType.U4)] FileShare share,
                                                     IntPtr securityAttributes, // optional SECURITY_ATTRIBUTES struct or IntPtr.Zero
                                                     [MarshalAs(UnmanagedType.U4)] FileMode creationDisposition,
                                                     [MarshalAs(UnmanagedType.U4)] FileAttributes flagsAndAttributes,
                                                     IntPtr templateFile);


        [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DeviceIoControl(  IntPtr hDevice,
                                                    uint dwIoControlCode,
                                                    ref long InBuffer,
                                                    int nInBufferSize,
                                                    ref long OutBuffer,
                                                    int nOutBufferSize,
                                                    ref int pBytesReturned,
                                                    [In] ref NativeOverlapped lpOverlapped);


        [DllImport("kernel32.dll", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        [SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseHandle(IntPtr hObject);

    }

}