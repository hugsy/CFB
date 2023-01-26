using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Helpers
{

    public class HookedDriverNotFoundException : Exception
    {
        public HookedDriverNotFoundException() : base("Cannot find hooked driver.") { }

        public HookedDriverNotFoundException(string message) : base(message) { }

        public HookedDriverNotFoundException(string message, Exception innerException) : base(message, innerException) { }
    }
}
