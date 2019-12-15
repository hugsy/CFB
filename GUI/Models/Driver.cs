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

        private bool _IsHooked;

        public Driver(String drivername)
        {
            DriverName = drivername;
            _IsHooked = false; 
            RefreshDeviceList();
            Devices = new List<Device>();
        }


        public string DriverName { get; set; }

        public bool IsHooked {
            get => _IsHooked; // todo: replace with ioctl
            set {
                if (!_IsHooked)
                    _IsHooked = Task.Run(() => App.BrokerSession.HookDriver(DriverName)).Result;
                else
                    _IsHooked = ! Task.Run(() => App.BrokerSession.UnhookDriver(DriverName)).Result;
            }
        }


        public void RefreshDeviceList()
        {
        }


    }

}
