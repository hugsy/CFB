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
using System.Runtime.InteropServices.WindowsRuntime;

using GUI.Helpers;
using GUI.Native;

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
        private Uri uri;
        


        public ConnectionManager()
        {
            uri = new Uri(LocalSettings.Values["IrpBrokerLocation"].ToString());
            ReinitializeSocket();
        }


        private void ReinitializeSocket()
        {
            ClientSocket = new StreamSocket();
            ClientSocket.Control.KeepAlive = true;
            //ClientSocket.Control.NoDelay = false;
            _Status = BrokerConnectionStatus.Disconnected;
        }

        public async Task<bool> Close()
        {
            ClientSocket.Dispose();
            _Status = BrokerConnectionStatus.Disconnected;
            return true;
        }


        public async Task<bool> Reconnect()
        {
            if (_Status == BrokerConnectionStatus.Connected || _Status == BrokerConnectionStatus.Connecting)
                return true;

            await Close();
            ReinitializeSocket();

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
                throw new Exception($"failed to connect to {uri.Host}:{uri.Port}");
            }
            return true;
        }


        public bool IsConnected
        {
            get => _Status == BrokerConnectionStatus.Connected;
        }

        public string TargetHost
        {
            get => uri.Host;
        }

        public int TargetPort
        {
            get => uri.Port;
        }

        public BrokerConnectionStatus Status
        {
            get => _Status;
        }


        public async Task send(byte[] message)
        {
            using (DataWriter writer = new DataWriter(ClientSocket.OutputStream))
            {
                writer.UnicodeEncoding = Windows.Storage.Streams.UnicodeEncoding.Utf8;
                writer.ByteOrder = Windows.Storage.Streams.ByteOrder.LittleEndian;

                try
                {
                    writer.WriteBytes(message);
                    await writer.StoreAsync();
                }
                catch (Exception exception)
                {
                    throw new Exception($"An error occured while sending message to {TargetHost}:{TargetPort}: {exception.Message}");
                }

                await writer.FlushAsync();
                writer.DetachStream();
            }
        }


        public async Task<byte[]> read()
        {
            List<byte> DataReceived;

            using (DataReader reader = new DataReader(ClientSocket.InputStream))
            {
                DataReceived = new List<byte>();

                reader.InputStreamOptions = Windows.Storage.Streams.InputStreamOptions.Partial;
                reader.UnicodeEncoding = Windows.Storage.Streams.UnicodeEncoding.Utf8;
                reader.ByteOrder = Windows.Storage.Streams.ByteOrder.LittleEndian;

                uint NomimalReadSize = 256;
                uint LeftToRead = NomimalReadSize;

                do
                {
                    // prefetch data from stream
                    await reader.LoadAsync(LeftToRead);
                    LeftToRead = reader.UnconsumedBufferLength;

                    // copy all the data received locally
                    for (uint i = 0; i < LeftToRead; i++)
                        DataReceived.Add(reader.ReadByte());

                    // did we prefetch everything? if so, break out
                    if (LeftToRead < NomimalReadSize)
                        break;
                }
                while (LeftToRead > 0);

                reader.DetachStream();
                return DataReceived.ToArray();
            }
        }


        private async Task<JObject> SendAndReceive(MessageType type, byte[] args = null)
        {
            BrokerMessage req = new BrokerMessage(type, args);
            await this.send(Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(req)));

            var RawResponse = await this.read();
            var JsonResponse = JObject.Parse( Encoding.Default.GetString(RawResponse) );

            return JsonResponse;
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

        public async Task<bool> HookDriver(String DriverName)
        {
            byte[] RawName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            JObject msg = await SendAndReceive(MessageType.HookDriver, RawName);
            return (bool)msg["header"]["success"];
            }

        public async Task<bool> UnhookDriver(String DriverName)
        {
            byte[] RawName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            JObject msg = await SendAndReceive(MessageType.UnhookDriver, RawName);
            return (bool)msg["header"]["success"];
        }

        /// <summary>
        /// Sends a "Start Monitoring" task to the broker
        /// </summary>
        /// <returns>True on success</returns>
        public async Task<bool> StartMonitoring()
        {
            JObject msg = await SendAndReceive(MessageType.EnableMonitoring);
            return (bool)msg["header"]["success"];
        }

        /// <summary>
        /// Sends a "Stop Monitoring" task to the broker
        /// </summary>
        /// <returns>True on success</returns>
        public async Task<bool> StopMonitoring()
        {
            JObject msg = await SendAndReceive(MessageType.DisableMonitoring);
            return (bool)msg["header"]["success"];
        }


        public async Task<JObject> GetDriverInfo(string DriverName)
        {
            byte[] RawDriverName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            JObject msg = await SendAndReceive(MessageType.GetDriverInfo, RawDriverName);
            bool is_success = (bool)msg["header"]["success"];

            if (!is_success)
            {
                uint gle = (uint)msg["header"]["gle"];
                switch ((Win32Error)gle)
                {
                    case Win32Error.ERROR_FILE_NOT_FOUND:
                        throw new HookedDriverNotFoundException(DriverName);

                    default:
                        throw new Exception($"GetDriverInfo('{DriverName}') failed: GLE=0x{gle}");
                }
            }
            return (JObject) msg["body"];
        }
    }
}
