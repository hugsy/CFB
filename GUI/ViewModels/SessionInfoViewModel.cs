using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.ViewModels
{
    public class SessionInfoViewModel : BindableBase
    {
        public SessionInfoViewModel() { }

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
                var DriverNames = new List<string>();
                var task = Task.Run(() =>App.Drivers.GetAsync());
                foreach (var driver in task.Result)
                {
                    if (driver.IsHooked)
                        DriverNames.Add($" → {driver.Name}");
                }
                return String.Join(System.Environment.NewLine, DriverNames);
            }
        }
    }
}
