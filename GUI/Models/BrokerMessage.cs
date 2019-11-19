using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GUI.Helpers;

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
    }
    

    public class BrokerMessageBody
    {
        public MessageType type;
        public uint data_length;
        public string data;
    }


    public class BrokerMessage
    {
        public BrokerMessageHeader headers;
        public BrokerMessageBody body;

        public BrokerMessage(MessageType type, byte[] args)
        {
            headers = new BrokerMessageHeader();
            body = new BrokerMessageBody();

            body.type = type;
            if(args == null)
            {
                body.data_length = 0;
                body.data = "";
            }
            else
            {
                body.data_length = (uint)args.Length;
                body.data = Utils.Base64Encode(args);
            }
        }
    }
}
