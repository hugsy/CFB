using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fuzzer
{
    class Wdm
    {
        public static uint FILE_DEVICE_UNKNOWN = 0x00000022;

        public static uint METHOD_BUFFERED = 0;
        public static uint METHOD_IN_DIRECT = 1;
        public static uint METHOD_OUT_DIRECT = 2;
        public static uint METHOD_NEITHER = 3;
        
        public static uint FILE_ANY_ACCESS = 0;
        public static uint FILE_SPECIAL_ACCESS = FILE_ANY_ACCESS;
        public static uint FILE_READ_ACCESS = 0x0001;
        public static uint FILE_WRITE_ACCESS = 0x0002;


        public static uint METHOD_FROM_CTL_CODE(uint CtrlCode)
        {
            return (uint)(CtrlCode & 3);
        }


        public static uint CTL_CODE(uint DeviceType, uint Function, uint Method, uint Access)
        {
            return (uint)(((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method));
        }


        public static uint DEVICE_TYPE_FROM_CTL_CODE(uint CtrlCode)
        {
            return (uint)((CtrlCode & 0xffff0000) >> 16);
        }


    }
}
