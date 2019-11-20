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

namespace GUI.Models
{
    /// <summary>
    /// This class manages the connection to the remote socket
    /// </summary>
    public class ConnectionManager
    {

        private ApplicationDataContainer LocalSettings = ApplicationData.Current.LocalSettings;

        private NamedPipeClientStream PipeClient;

        private StreamSocket ClientSocket;


        public ConnectionManager()
        {
            ClientSocket = new StreamSocket();
        }

        public async void Reconnect()
        {
            var uri = new Uri(LocalSettings.Values["IrpBrokerLocation"].ToString());
            var host = uri.Host;
            var path = uri.AbsolutePath.Replace("/pipe/", "");
            //PipeClient = new NamedPipeClientStream(host, path, PipeDirection.InOut, PipeOptions.None);
            //PipeClient.Connect();
            var port = uri.Port;
            HostName serverHost = new HostName(host);
            string serverPort = port.ToString();
            await ClientSocket.ConnectAsync(serverHost, serverPort);
        }

        public bool IsConnected
        {
            get => PipeClient.IsConnected;
        }

        public void Close() =>
            PipeClient.Close();


        private async Task<JObject> SendAndReceive(MessageType type, byte[] args = null)
        {
            Stream OutgoingStream = ClientSocket.OutputStream.AsStreamForWrite();
            //StreamWriter writer = new StreamWriter(streamOut);
            //string request = cm;
            //await writer.WriteLineAsync(request);
            //await writer.FlushAsync();
            BrokerMessage req = new BrokerMessage(type, args);
            using (StreamWriter sw = new StreamWriter(OutgoingStream))
            {
                sw.Write(JsonConvert.SerializeObject(req));
                sw.Flush();
            }
            //log("send request successful");


            Stream IngoingStream = ClientSocket.InputStream.AsStreamForRead();
            //StreamReader reader = new StreamReader(streamIn);
            //log("read response from server:" + reader.ReadLineAsync());

            string res;
            using (StreamReader sr = new StreamReader(IngoingStream))
            {
                res = sr.ReadToEnd();
            }

            return JObject.Parse(res);
        }


        public async IEnumerable<string> EnumerateDrivers()
        {
            JObject msg =  await SendAndReceive(MessageType.EnumerateDrivers);
            if ((bool)msg["header"]["success"] == true)
            {
                foreach (string driver_name in (JArray)msg["body"]["drivers"])
                {
                    yield return driver_name;
                }
            }
            
        }

    }
}
