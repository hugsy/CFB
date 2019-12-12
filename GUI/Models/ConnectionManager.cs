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
using System.Threading;
using Windows.Storage.Streams;
using System.Text;

namespace GUI.Models
{
    public enum BrokerConnectionStatus
    {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    }


    /// <summary>
    /// This class manages the connection to the remote socket
    /// </summary>
    public class ConnectionManager
    {
        private ApplicationDataContainer LocalSettings = ApplicationData.Current.LocalSettings;
        private StreamSocket ClientSocket;
        private BrokerConnectionStatus _Status;


        public ConnectionManager()
        {
            ClientSocket = new StreamSocket();
            ClientSocket.Control.NoDelay = true;
            ClientSocket.Control.KeepAlive = true;
            _Status = BrokerConnectionStatus.Disconnected;
        }


        public async void Reconnect()
        {
            if (_Status == BrokerConnectionStatus.Connected || _Status == BrokerConnectionStatus.Connecting)
                return;

            var uri = new Uri(LocalSettings.Values["IrpBrokerLocation"].ToString());
            _Status = BrokerConnectionStatus.Connecting;
            try
            {
                await ClientSocket.ConnectAsync(
                    new HostName(uri.Host), 
                    uri.Port.ToString(),
                    SocketProtectionLevel.PlainSocket
                );
                _Status = BrokerConnectionStatus.Connected;
            }
            catch(Exception)
            {
                _Status = BrokerConnectionStatus.Disconnected;
                throw new Exception("failed to connect");
            }
        }


        public bool IsConnected
        {
            get => _Status == BrokerConnectionStatus.Connected;
        }

        public string TargetHost
        {
            get => new Uri( ApplicationData.Current.LocalSettings.Values["IrpBrokerLocation"].ToString() ).Host;
        }

        public int TargetPort
        {
            get => new Uri(ApplicationData.Current.LocalSettings.Values["IrpBrokerLocation"].ToString()).Port;
        }


        public void Close()
        {
            ClientSocket.Dispose();
            _Status = BrokerConnectionStatus.Disconnected;
        }


        private async Task<JObject> SendAndReceive(MessageType type, byte[] args = null)
        {
            if (!IsConnected)
                Reconnect();

            BrokerMessage req = new BrokerMessage(type, args);
            using (var sw = new StreamWriter(ClientSocket.OutputStream.AsStreamForWrite()))
            {
                await sw.WriteAsync(JsonConvert.SerializeObject(req));
            }


            var sb = new StringBuilder();

            using (var sr = new StreamReader(ClientSocket.InputStream.AsStreamForRead()))
            {
                int l = 0;
                while (true)
                {
                    var buffer = new char[1];
                    var nb_read = await sr.ReadAsync(buffer, 0, 1);
                    
                    if (nb_read <= 0)
                        throw new Exception("Server disconnected");
                    
                    sb.Append(buffer);

                    if (buffer[0] == '{')
                        l++;
                    if (buffer[0] == '}')
                        l--;
                    if (l == 0)
                        break;


                    if (nb_read == buffer.Length)
                        continue;
                    
                    break;
                }
            }

            return JObject.Parse(sb.ToString());
        }

        
        public async Task<List<Driver>> EnumerateDrivers()
        {
            JObject msg = await SendAndReceive(MessageType.EnumerateDrivers);
            bool is_success = (bool)msg["header"]["success"];

            if (!is_success)
                throw new Exception("SendAndReceive(EnumerateDrivers) operation returned " + is_success);


            List<Driver> drivers = new List<Driver>();
            foreach (string driver_name in (JArray)msg["body"]["drivers"])
                drivers.Add( new Driver(driver_name) );

            return drivers;
        }
    }
}
