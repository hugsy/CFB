using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GUI.Helpers;
using GUI.Models;
using GUI.Native;

namespace GUI.ViewModels
{

    public class DriverViewModel : BindableBase
    {
        public DriverViewModel(Driver model)
        {
            Model = model;
        }


        private Driver _model;


        public Driver Model
        {
            get => _model;
            set
            {
                if (_model != value)
                {
                    _model = value;
                    //Task.Run(RefreshDriverAsync);
                    OnPropertyChanged();
                }
            }
        }

        public override string ToString()
            => Name;

        public string Name
        {
            get => Model.Name;
        }

        public bool IsEnabled
        {
            get => Model.IsHooked && Model.IsEnabled;
        }

        public ulong NumberOfRequestIntercepted
        {
            get => Model.NumberOfRequestIntercepted;
        }


        public ulong Address
        {
            get => Model.Address;
        }


        public bool IsHooked
        {
            get => Model.IsHooked;
        }

        
        private void PropagateChangesToView()
        {
            Task.Run(RefreshDriverAsync);
            OnPropertyChanged();
        }


        public bool TryToggleHookStatus()
        {
            bool data_changed = IsHooked ?
                Task.Run(() => App.BrokerSession.UnhookDriver(Name)).Result :
                Task.Run(() => App.BrokerSession.HookDriver(Name)).Result;

            if (data_changed)
                PropagateChangesToView();

            return data_changed;
        }


        public async void RefreshDriverAsync()
        {
            var msg = await App.BrokerSession.GetDriverInfo(Name);

            if (!msg.header.is_success)
            {
                if (msg.header.gle == Win32Error.ERROR_FILE_NOT_FOUND)
                {
                    Model.IsEnabled = false;
                    Model.IsHooked = false;
                    Model.Address = 0;
                    Model.NumberOfRequestIntercepted = 0;
                }
                else
                {
                    Model.IsEnabled = false;
                    Model.IsHooked = false;
                    Model.Address = ~-1;
                    Model.NumberOfRequestIntercepted = ~-1;
                    Model.Name = $"Error 0x{msg.header.gle}";
                }
            }
            else
            {
                var driver = msg.body.driver;
                if(!String.Equals(driver.Name, Model.Name, StringComparison.OrdinalIgnoreCase))
                    throw new Exception("unexpected driver info");
                Model.IsHooked = true;
                Model.IsEnabled = driver.IsEnabled;
                Model.Address = driver.Address;
                Model.NumberOfRequestIntercepted = driver.NumberOfRequestIntercepted;
            }
        }
    }
}
