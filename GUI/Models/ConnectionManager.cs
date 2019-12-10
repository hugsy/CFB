using System;
using System.Collections.Generic;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage;
using System.IO;
using System.IO.Pipes;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using System.Threading.Tasks;
using Windows.UI.Popups;

namespace GUI.Models
{
    /// <summary>
    /// This class manages the connection to the remote socket
    /// </summary>
    public class ConnectionManager
    {

        private ApplicationDataContainer LocalSettings = ApplicationData.Current.LocalSettings;

        private StreamSocket ClientSocket;
        private bool _IsConnected;


        public ConnectionManager()
        {
            ClientSocket = new StreamSocket();
            ClientSocket.Control.KeepAlive = true;
            _IsConnected = false;

        }

        public async void Reconnect()
        {
            var uri = new Uri(LocalSettings.Values["IrpBrokerLocation"].ToString());
            await ClientSocket.ConnectAsync(new HostName(uri.Host), uri.Port.ToString());
            _IsConnected = true;
        }


        public bool IsConnected
        {
            get => _IsConnected;
        }


        public void Close()
        {
            ClientSocket.Dispose();
            _IsConnected = false;
        }


        private JObject SendAndReceive(MessageType type, byte[] args = null)
        {
            if (!IsConnected)
                Reconnect();

            if (!IsConnected)
                throw new System.InvalidOperationException("Server is unreachable");

            string res = "";

            Stream OutgoingStream = ClientSocket.OutputStream.AsStreamForWrite();
            BrokerMessage req = new BrokerMessage(type, args);
            using (StreamWriter sw = new StreamWriter(OutgoingStream))
            {
                sw.Write(JsonConvert.SerializeObject(req));
                sw.Flush();
            }

            Stream IngoingStream = ClientSocket.InputStream.AsStreamForRead();
                            
            using (StreamReader sr = new StreamReader(IngoingStream))
            {
                res = sr.ReadToEnd();
            }

            return JObject.Parse(res);
        }

        
        public IEnumerable<Driver> EnumerateDrivers()
        {
            JObject msg = SendAndReceive(MessageType.EnumerateDrivers);
            if ((bool)msg["header"]["success"] == true)
            {
                foreach (string driver_name in (JArray)msg["body"]["drivers"])
                {
                    yield return new Driver(driver_name);
                }
            }
        }
    }
}
