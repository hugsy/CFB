using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Models
{
    public class Driver 
    {

        public Driver(String drivername)
        {
            DriverName = drivername;
            IsHooked = false; // todo: replace with ioctl
            RefreshDeviceList();
        }


        public string DriverName { get; set; }

        public bool IsHooked { get; set; }


        public List<Device> Devices = new List<Device>();

        public void RefreshDeviceList()
        {
        }


    }

}
