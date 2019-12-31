using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GUI.Helpers;
using GUI.Native;
using Newtonsoft.Json.Linq;

namespace GUI.Models
{
    /// <summary>
    /// The different message types to the Broker
    /// The values must reflect *exactly* the values defined in Broker\Task.h
    /// </summary>
    public enum MessageType : uint
    {
        IoctlResponse                = 1, // useless now, todo replace with something more useful
        HookDriver                   = 2,
        UnhookDriver                 = 3,
        GetDriverInfo                = 4,
        NumberOfDriver               = 5,
        NotifyEventHandle            = 6, // don't use, todo remove
        EnableMonitoring             = 7,
        DisableMonitoring            = 8,
        GetInterceptedIrps           = 9,
        ReplayIrp                    = 10,
        StoreTestCase                = 11,
        EnumerateDrivers             = 12,
        EnableDriver                 = 13,
        DisableDriver                = 14,
        GetNamesOfHookedDrivers      = 15,
    };


    public class BrokerMessageHeader
    {
        public BrokerMessageHeader() { }

        public Win32Error gle;
        public bool is_success;
        public MessageType type;
    }
    

    public class BrokerMessageBody
    {
        public BrokerMessageBody() { }

        // used by requests
        public uint param_length;
        public string param;

        // used by EnumerateDrivers && GetNamesOfHookedDrivers
        public List<String> drivers;  

        // used by GetDriverInfo
        public Driver driver;

        //used by GetInterceptedIrps
        public uint nb_irps;
        public List<Irp> irps;

        // used by ReplayIrp
        /// request
        //public string device_name;
        //public int ioctl_code;
        //public byte[] input_buffer;
        //public int input_buffer_length;
        /// response
        public byte[] output_buffer;
        public int output_buffer_length;
    }


    public class BrokerMessage
    {
        public BrokerMessageHeader header;
        public BrokerMessageBody body;

        public BrokerMessage() { }
       

        public BrokerMessage(MessageType type, byte[] args=null)
        {
            header = new BrokerMessageHeader();
            header.type = type;

            body = new BrokerMessageBody();
            if(args == null)
            {
                body.param_length = 0;
                body.param = "";
            }
            else
            {
                body.param_length = (uint)args.Length;
                body.param = Utils.Base64Encode(args);
            }
        }
    }
}
