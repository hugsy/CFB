using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Models
{
    public class Driver 
    {
        public List<Device> Devices;

        public Driver(String drivername)
        {
            DriverName = drivername;
            _IsHooked = false;
            NumberOfRequestIntercepted = 0;
            Address = 0;
            Devices = new List<Device>();
            RefreshDriverInfoAsync();
        }


        public string DriverName { get; private set; }
        public ulong Address { get; private set; }
        public ulong NumberOfRequestIntercepted { get; private set; }

        private bool _IsHooked;
        public bool IsHooked {
            get => _IsHooked;

            set {
                bool data_changed = false;
                if (_IsHooked == false)
                    data_changed = Task.Run(() => App.BrokerSession.HookDriver(DriverName)).Result;
                else
                    data_changed = Task.Run(() => App.BrokerSession.UnhookDriver(DriverName)).Result;

                if (data_changed)
                    RefreshDriverInfoAsync();
            }
        }


        public void RefreshDriverInfoAsync()
        {
            Devices.Clear();
            try 
            {
                var js_body = Task.Run(() => App.BrokerSession.GetDriverInfo(DriverName)).Result;
                var js_data = js_body["data"];
                _IsHooked = (bool)js_data["Enabled"];
                Address = (ulong)js_data["DriverAddress"];
                NumberOfRequestIntercepted = (ulong)js_data["NumberOfRequestIntercepted"];
            }
            catch(Exception)
            {
                _IsHooked = false;
                Address = 0;
                NumberOfRequestIntercepted = 0;
            }
        }
    }

}
