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
    public enum MessageType : uint
    {
        IoctlResponse = 1,
        HookDriver = 2,
        UnhookDriver = 3,
        GetDriverInfo = 4,
        NumberOfDriver = 5,
        NotifyEventHandle = 6,
        EnableMonitoring = 7,
        DisableMonitoring = 8,
        GetInterceptedIrps = 9,
        ReplayIrp = 10,
        StoreTestCase = 11,
        EnumerateDrivers = 12
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

        // used by EnumerateDrivers
        public List<String> drivers;

        // used by GetDriverInfo
        public Driver driver;
    }


    public class BrokerMessage
    {
        public BrokerMessageHeader header;
        public BrokerMessageBody body;

        public BrokerMessage() 
        { }


        public BrokerMessage(MessageType type) : this(type, null)
        { }
        

        public BrokerMessage(MessageType type, byte[] args)
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
