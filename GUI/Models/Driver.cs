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
        public uint Address { get; private set; }
        public uint NumberOfRequestIntercepted { get; private set; }

        private bool _IsHooked;
        public bool IsHooked {
            get => _IsHooked;

            private set {
                bool data_changed = false;
                if (_IsHooked == false)
                    data_changed = Task.Run(() => App.BrokerSession.HookDriver(DriverName)).Result;
                else
                    data_changed = Task.Run(() => App.BrokerSession.UnhookDriver(DriverName)).Result;

                if (data_changed)
                    RefreshDriverInfoAsync();
            }
        }


        private void RefreshDriverInfoAsync()
        {
            Devices.Clear();
            try 
            {
                var js_body = Task.Run(() => App.BrokerSession.GetDriverInfo(DriverName)).Result;
                _IsHooked = (bool)js_body["Enabled"];
                Address = (uint)js_body["Address"];
                NumberOfRequestIntercepted = (uint)js_body["NumberOfRequestIntercepted"];
            }
            catch(Exception)
            {

            }
        }
    }

}
