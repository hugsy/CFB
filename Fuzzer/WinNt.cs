using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fuzzer
{
    class WinNt
    {
        public static uint STANDARD_RIGHTS_REQUIRED = 0x000F0000;
        public static uint DELETE = 0x00010000;
        public static uint SERVICE_KERNEL_DRIVER = 0x00000001;
        public static uint SERVICE_DEMAND_START = 0x00000003;

        public static uint SERVICE_ERROR_IGNORE = 0x00000000;
        public static uint SERVICE_ERROR_NORMAL = 0x00000001;
        public static uint SERVICE_ERROR_SEVERE = 0x00000002;
        public static uint SERVICE_ERROR_CRITICAL = 0x00000003;

        public static uint ERROR_SERVICE_EXISTS = 1073;
    }
}
