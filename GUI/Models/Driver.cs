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
            _IsEnabled = false;
            NumberOfRequestIntercepted = 0;
            Address = 0;
            Devices = new List<Device>();
            RefreshDriverInfoAsync();
        }


        public string DriverName { get; private set; }
        public ulong Address { get; private set; }
        public ulong NumberOfRequestIntercepted { get; private set; }

        private bool _IsEnabled;
        public bool IsEnabled
        {
            get => _IsHooked && _IsEnabled;
            // todo: handle setter
        }

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

                // if we didn't get an exception with NTSTATUS = STATUS_OBJECT_NAME_NOT_FOUND(0xc0000034)
                // it means the driver is hooked
                _IsHooked = true;

                // use the json object to populate the other attributes
                var js_data = js_body["data"];
                _IsEnabled = (bool)js_data["Enabled"];
                Address = (ulong)js_data["DriverAddress"];
                NumberOfRequestIntercepted = (ulong)js_data["NumberOfRequestIntercepted"];
            }
            catch(Exception) // todo: use HookedDriverNotFoundException()
            {
                _IsEnabled = false;
                _IsHooked = false;
                Address = 0;
                NumberOfRequestIntercepted = 0;
            }
        }
    }

}
