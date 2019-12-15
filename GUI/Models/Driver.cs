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
            get
            {
                try
                {
                    var js = Task.Run(() => App.BrokerSession.GetDriverInfo(DriverName));
                    _IsHooked = (bool)js.Result["data"]["Enabled"];
                }
                catch(Exception)
                {
                    // if the driver is not hooked, DeviceIoctlControl() will return FALSE, upon which GetDriverInfo() will generate an Exception
                    // todo: make better exception for this case
                    _IsHooked = false;
                }

                return _IsHooked;
            }

            set {
                if (_IsHooked == false)
                {
                    // try to hook
                    var res = Task.Run(() => App.BrokerSession.HookDriver(DriverName)).Result;
                    if (res)
                        // if the operation was successful, update the field
                        _IsHooked = true;
                }
                else
                {
                    // try to unhook
                    var res = Task.Run(() => App.BrokerSession.UnhookDriver(DriverName)).Result;
                    if (res)
                        // if the operation was successful, update the field
                        _IsHooked = false;
                }
            }
        }


        public void RefreshDeviceList()
        {
        }


    }

}
