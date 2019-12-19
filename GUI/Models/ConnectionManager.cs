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
    /// 
    /// This class manages the connection and message to the remote broker
    /// 
    /// </summary>
    public class ConnectionManager
    {
        private readonly ApplicationDataContainer LocalSettings = ApplicationData.Current.LocalSettings;
        private StreamSocket ClientSocket;
        private BrokerConnectionStatus _Status;
        private readonly Uri uri;
        private static readonly SemaphoreSlim sem = new SemaphoreSlim(1, 1);



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
            await Task.Run( () => ClientSocket.Dispose() );
            _Status = BrokerConnectionStatus.Disconnected;
            return true;
        }


        public async Task<bool> Reconnect()
        {
            if (_Status == BrokerConnectionStatus.Connected || _Status == BrokerConnectionStatus.Connecting)
                return true;

            _ = await Close();
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


        public async Task SendBytes(byte[] message)
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


        public async Task<byte[]> ReceiveBytes()
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
                    {
                        byte b = reader.ReadByte();
                        DataReceived.Add(b);
                    }

                    // did we prefetch everything? if so, break out
                    if (LeftToRead < NomimalReadSize)
                        break;
                }
                while (LeftToRead > 0);

                reader.DetachStream();
                return DataReceived.ToArray();
            }
        }


        private async Task<BrokerMessage> SendAndReceive(MessageType type, byte[] args = null)
        {
            BrokerMessage req, res;
            byte[] RawResponse;
            req = new BrokerMessage(type, args);

            await sem.WaitAsync();
            try
            {
                await this.SendBytes(
                    Encoding.UTF8.GetBytes(
                        JsonConvert.SerializeObject(
                            req,
                            Newtonsoft.Json.Formatting.None,
                            new JsonSerializerSettings { NullValueHandling = NullValueHandling.Ignore }
                        )
                    )
                );
                RawResponse = await this.ReceiveBytes();
            }
            finally
            {
                sem.Release();
            }

            res = JsonConvert.DeserializeObject<BrokerMessage>(Encoding.Default.GetString(RawResponse));
            return res;
        }

        
        public async Task<List<Driver>> EnumerateDrivers()
        {
            var msg = await SendAndReceive(MessageType.EnumerateDrivers);
            if (!msg.header.is_success)
                // should never happen, EnumerateDrivers success always return true even if there is no driver
                throw new Exception($"SendAndReceive({nameof(MessageType.EnumerateDrivers)}) operation returned FALSE: 0x{msg.header.gle:x}");
                
            List<Driver> drivers = new List<Driver>();

            foreach (string driver_name in msg.body.drivers)
                drivers.Add( new Driver(driver_name) );

            return drivers;
        }


        public async Task<bool> HookDriver(String DriverName)
        {
            byte[] RawName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            var msg = await SendAndReceive(MessageType.HookDriver, RawName);
            return msg.header.is_success;
        }


        public async Task<bool> UnhookDriver(String DriverName)
        {
            byte[] RawName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            var msg = await SendAndReceive(MessageType.UnhookDriver, RawName);
            return msg.header.is_success;
        }


        /// <summary>
        /// Sends a "Start Monitoring" task to the broker
        /// </summary>
        /// <returns>True on success</returns>
        public async Task<bool> StartMonitoring()
        {
            var msg = await SendAndReceive(MessageType.EnableMonitoring);
            return msg.header.is_success;
        }


        /// <summary>
        /// Sends a "Stop Monitoring" task to the broker
        /// </summary>
        /// <returns>True on success</returns>
        public async Task<bool> StopMonitoring()
        {
            var msg = await SendAndReceive(MessageType.DisableMonitoring);
            return msg.header.is_success;
        }


        public async Task<BrokerMessage> GetDriverInfo(string DriverName)
        {
            byte[] RawDriverName = Encoding.Unicode.GetBytes($"{DriverName.ToLower()}\x00");
            var msg = await SendAndReceive(MessageType.GetDriverInfo, RawDriverName);
            return msg;
        }
    }
}
