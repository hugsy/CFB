using System;
using System.Diagnostics;

namespace Fuzzer
{
    class Utils
    {

        public static string GetProcessById(uint ProcessId)
        {
            string Res = "";

            try
            {
                Process p = Process.GetProcessById(( int )ProcessId);
                Res = p.ProcessName;
            }
            catch
            {
                Res = "";
            }

            return Res;
        }

    }
}
