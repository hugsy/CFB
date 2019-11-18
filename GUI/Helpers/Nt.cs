using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;


using GUI.Native;



namespace GUI.Helpers
{
    class Nt
    {
        /// <summary>
        /// Enumerates object name from the Windows Object Directory from the root path specified
        /// </summary>
        /// <param name="RootPath"></param>
        /// <returns>A generator of string with Objects name</returns>
        public static IEnumerable<string> EnumerateDirectoryObjects(string RootPath = "\\")
        {
            /*
#pragma warning disable IDE0018 // Inline variable declaration
            SafeFileHandle Handle;
#pragma warning restore IDE0018 // Inline variable declaration            
            
            do
            {
                var ObjAttr = new Win32.OBJECT_ATTRIBUTES(RootPath, Win32.OBJ_CASE_INSENSITIVE);

                var Status = Win32.NtOpenDirectoryObject(out Handle, Win32.DIRECTORY_QUERY | Win32.DIRECTORY_TRAVERSE, ref ObjAttr);
                if (Status != 0)
                {
                    yield break;
                }


                int RawBufferSize = 128 * 1024;
                IntPtr RawBuffer = Marshal.AllocHGlobal(RawBufferSize);
                int ObjDirInfoStructSize = Marshal.SizeOf(typeof(Win32.OBJECT_DIRECTORY_INFORMATION));
                uint Context = 0;

                Status = Win32.NtQueryDirectoryObject(Handle, RawBuffer, RawBufferSize, false, true, ref Context, out uint ReturnLength);
                if (Status == 0)
                {
                    for (int i = 0; i < Context; i++)
                    {
                        IntPtr Addr = RawBuffer + i * ObjDirInfoStructSize;
                        var ObjectDirectoryInformation = (Win32.OBJECT_DIRECTORY_INFORMATION)Marshal.PtrToStructure(Addr, typeof(Win32.OBJECT_DIRECTORY_INFORMATION));
                        yield return ObjectDirectoryInformation.Name.ToString();
                    }
                }

                Marshal.FreeHGlobal(RawBuffer);
                Handle.Dispose();
            }
            while (false);
            */

            yield return "\\driver\\foobar";
            yield return "\\driver\\foobaz";
        }


    }
}

