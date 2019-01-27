using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fuzzer
{
    public static class Settings
    {

        //
        // private settings
        //

        private static int __verbosity = 0;
        private static bool __autofuzzirp = false;



        //
        // public settings
        //

        public static int VerbosityLevel
        {
            get
            {
                return __verbosity;
            }

            set
            {
                __verbosity = value;
            }
        }

        public static bool AutoFuzzNewIrp
        {
            get
            {
                return __autofuzzirp;
            }

            set
            {
                __autofuzzirp = value;
            }
        }

        public static string CfbDevicePath
        {
            get
            {
                return "\\\\.\\IrpDumper";
            }
        }


        public static string CfbDriverFilename
        {
            get
            {
                return "IrpDumper.sys";
            }
        }


        public static string CfbDriverShortName
        {
            get
            {
                return "IrpDumper";
            }
        }


        public static string CfbDriverDescription
        {
            get
            {
                return "CFB IRP Dumper Driver";
            }
        }


    }

    
}
