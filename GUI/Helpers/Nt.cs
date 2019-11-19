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
        public static IEnumerable<string> EnumerateDriverObjects(string RootPath = "\\")
        {
            yield return "\\driver\\foobar";
            yield return "\\driver\\foobaz";
        }


    }
}

