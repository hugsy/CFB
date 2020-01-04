using GUI.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.ViewModels
{
    public class SessionInfoViewModel : BindableBase
    {
        private uint _versionMajor = 0;
        private uint _versionMinor = 0;
        private string _versionBuild = "0";
        private uint _cpuArchitecture = 0;
        private uint _cpuNumber = 0;

        private List<Driver> Drivers;


        public SessionInfoViewModel()
        {
            Drivers = App.Drivers.Get().ToList();
            RefreshValues();
        }


        public async Task RefreshValues()
        {
            var msg = await App.BrokerSession.GetOsInfo();
            _versionMajor = msg.version_major;
            _versionMinor = msg.version_minor;
            _versionBuild = msg.version_build;
            _cpuArchitecture = msg.cpu_arch;
            _cpuNumber = msg.cpu_num;
        }


        private bool _isLoading = false;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }

        public int NumberOfIrpsCaptured
        {
            get => App.Irps.Count();
        }

        public int NumberOfDrivers
        {
            get => App.Drivers.Count(false, false);
        }

        public string NumberOfDriversString
        {
            get => $"{NumberOfDrivers} driver(s) found";
        }

        public int NumberOfDriversHooked
        {
            get => App.Drivers.Count(true, false);
        }

        public string NumberOfDriversHookedString
        {
            get => $"{NumberOfDriversHooked:d} driver(s) hooked";
        }


        public bool IsConnected
        {
            get => App.BrokerSession.IsConnected;
        }


        public string ConnectionStatus
        {
            get => IsConnected ? "Connected" : "Disconnected";
        }


        public string HookedDriverList
        {
            get
            {
                if (!IsConnected) 
                    return "";

                var DriverNames = GetHookedDriverList();
                return String.Join(System.Environment.NewLine, DriverNames);
            }
        }

        private List<string> GetHookedDriverList()
        {
            var DriverNames = new List<string>();
            var drivers = App.Drivers.Get();
            foreach (var driver in Drivers)
            {
                if (driver.IsHooked)
                    DriverNames.Add($" → {driver.Name}");
            }
            return DriverNames;
        }


        public string RemoteOsVersion
        {
            get => $"{_versionMajor:d}.{_versionMinor:d}.{_versionBuild:s}";
        }

        public string RemoteNumberOfProcessor
        {
            get => $"{_cpuNumber} logical processor(s)";
        }

        public string RemoteCpuArchitecture
        {
            get
            {
                switch(_cpuArchitecture)
                {
                    case 0: return "PROCESSOR_ARCHITECTURE_INTEL";
                    case 5: return "PROCESSOR_ARCHITECTURE_ARM";
                    case 6: return "PROCESSOR_ARCHITECTURE_IA64";
                    case 9: return "PROCESSOR_ARCHITECTURE_AMD64";
                    case 12: return "PROCESSOR_ARCHITECTURE_ARM64";
                }
                return "PROCESSOR_ARCHITECTURE_UNKNOWN";
            }
        }
    }
}
