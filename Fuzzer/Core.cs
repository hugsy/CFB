using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Fuzzer
{
    /// <summary>
    /// Abstraction class for interacting with the low-level functions from Core.dll
    /// </summary>
    class Core
    {

        [DllImport("Core.dll", EntryPoint = "DllMain", CharSet = CharSet.Unicode)]
        private static extern bool HasPrivilege(String PrivilegeName, IntPtr lpHasPriv);

        public static bool HasPrivilege(String PrivName)
        {
            IntPtr lpHasPriv = new IntPtr(0);

            if (!HasPrivilege(PrivName, lpHasPriv))
            {
                throw new Exception("HasPrivilege() failed");
            }

            return (int)lpHasPriv != 0;
        }

        

    }
}
