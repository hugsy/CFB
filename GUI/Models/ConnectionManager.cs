using Microsoft.Win32.SafeHandles;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using System.IO;
using System.IO.Pipes;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;

namespace GUI.Models
{
    /// <summary>
    /// This class manages the connection to the remote pipe
    /// </summary>
    public class ConnectionManager
    {

        private ApplicationDataContainer LocalSettings = ApplicationData.Current.LocalSettings;

        private NamedPipeClientStream PipeClient;

        public ConnectionManager()
        {

        }

        public void Reconnect()
        {
            if (PipeClient != null && PipeClient.IsConnected)
                PipeClient.Close();

            var uri = new Uri(LocalSettings.Values["IrpBrokerLocation"].ToString());
            var host = uri.Host;
            var path = uri.AbsolutePath.Replace("/pipe/", "");
            PipeClient = new NamedPipeClientStream(host, path, PipeDirection.InOut, PipeOptions.None);
            PipeClient.Connect();
        }

        public bool IsConnected
        {
            get => PipeClient.IsConnected;
        }

        public void Close() =>
            PipeClient.Close();


        private JObject SendAndReceive(MessageType type, byte[] args = null)
        {
            if (PipeClient == null || !IsConnected)
            {
                Reconnect();
            }

            BrokerMessage req = new BrokerMessage(type, args);
            using (StreamWriter sw = new StreamWriter(PipeClient))
            {
                sw.Write(JsonConvert.SerializeObject(req));
                sw.Flush();
            }

            string res;
            using (StreamReader sr = new StreamReader(PipeClient))
            {
                res = sr.ReadToEnd();
            }

            return JObject.Parse(res);
        }


        public IEnumerable<string> EnumerateDrivers()
        {
            JObject msg = SendAndReceive(MessageType.EnumerateDrivers);
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
