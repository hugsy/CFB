using GUI.Models;
using Microsoft.Toolkit.Uwp.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.System.Threading;

namespace GUI.ViewModels
{
    public class SessionInfoViewModel : BindableBase
    {
        private uint _versionMajor = 0;
        private uint _versionMinor = 0;
        private string _versionBuild = "0";
        private uint _cpuArchitecture = 0;
        private uint _cpuNumber = 0;
        private string _userName = "";
        private string _integrityLevel = "";
        private uint _processId = 0;
        private float _version = 0;

        private ThreadPoolTimer PeriodicTimer;


        public SessionInfoViewModel(){}


        public void StartPeriodicTimer()
        {
            TimeSpan period = TimeSpan.FromSeconds(2);
            PeriodicTimer = ThreadPoolTimer.CreatePeriodicTimer(
                async (src) => {
                    var success = await RefreshValues();
                    if (!success) 
                        src.Cancel(); 
                }, 
                period
            );
        }


        public async Task<bool> RefreshValues()
        {
            if (!App.BrokerSession.IsConnected)
                return false;

            var msg = await App.BrokerSession.GetOsInfo();

            await DispatcherHelper.ExecuteOnUIThreadAsync(() =>
            {
                RemoteMajorVersion = msg.version_major;
                RemoteMinorVersion = msg.version_minor;
                RemoteBuildVersion = msg.version_build;

                RemoteCpuArchitecture = msg.cpu_arch;
                RemoteNumberOfProcessor = msg.cpu_num;

                UserName = msg.username;
                IntegrityLevel = msg.integrity;
                ProcessId = msg.pid;
                Version = msg.version;
            });

            return true;
        }



        public void StopPeriodicTimer()
        {
            PeriodicTimer.Cancel();
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

                var DriverNames = new List<string>();
                foreach (var driver in App.Drivers.Get())
                {
                    if (driver.IsHooked)
                        DriverNames.Add($" → {driver.Name}");
                }
                return String.Join(System.Environment.NewLine, DriverNames);
            }
        }


        public uint RemoteMajorVersion
        {
            get => _versionMajor;
            set
            {
                Set(ref _versionMajor, value);
                OnPropertyChanged("RemoteOsVersion");
            }
        }

        public uint RemoteMinorVersion
        {
            get => _versionMinor;
            set {
                Set(ref _versionMinor, value);
                OnPropertyChanged("RemoteOsVersion");
            }
        }

        public string RemoteBuildVersion
        {
            get => _versionBuild;
            set
            {
                Set(ref _versionBuild, value);
                OnPropertyChanged("RemoteOsVersion");
            }
        }

        public string RemoteOsVersion
        {
            get => $"WinVer={RemoteMajorVersion}.{RemoteMinorVersion}.{RemoteBuildVersion}";
        }

        public uint RemoteNumberOfProcessor
        {
            get => _cpuNumber;
            set { 
                Set(ref _cpuNumber, value);
                OnPropertyChanged("RemoteNumberOfProcessorString");
            }
        }

        public string RemoteNumberOfProcessorString
        {
            get => $"{RemoteNumberOfProcessor:d} logical processor(s)";
        }

        public uint RemoteCpuArchitecture
        {
            get => _cpuArchitecture;
            set { 
                Set(ref _cpuArchitecture, value);
                OnPropertyChanged("RemoteCpuArchitectureString");
            }
        }


        public string RemoteCpuArchitectureString
        {
            get
            {
                switch(RemoteCpuArchitecture)
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

        public string UserName
        {
            get => _userName;
            set => Set(ref _userName, value);
        }

        public string IntegrityLevel
        {
            get => _integrityLevel;
            set => Set(ref _integrityLevel, value);
        }

        public uint ProcessId
        {
            get => _processId;
            set {
                Set(ref _processId, value);
                OnPropertyChanged("ProcessInfo");
            }
        }

        public float Version
        {
            get => _version;
            set {
                Set(ref _version, value);
                OnPropertyChanged("ProcessInfo");
            }
        }

        public string ProcessInfo
        {
            get => $"Broker.exe v.{Version.ToString("n2")} running as PID={ProcessId}";
        }
    }
}
