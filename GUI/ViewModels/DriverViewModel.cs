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
                    Task.Run(RefreshDriverAsync);
                    OnPropertyChanged(string.Empty);
                }
            }
        }

        public string Name
        {
            get => Model.Name;
        }

        public bool IsEnabled
        {
            get => Model.IsHooked && Model.IsEnabled;
            // todo: handle setter
        }


        public bool IsHooked
        {
            get => Model.IsHooked;

            set
            {
                bool data_changed = false;
                if (IsHooked == false)
                    data_changed = Task.Run(() => App.BrokerSession.HookDriver(Name)).Result;
                else
                    data_changed = Task.Run(() => App.BrokerSession.UnhookDriver(Name)).Result;

                //if (data_changed)
                //    Task.Run(RefreshDriverAsync);
            }
        }


        private bool _isLoading;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }



        public async Task RefreshDriverAsync()
        {
            var msg = await App.BrokerSession.GetDriverInfo(Name);

            if (!msg.header.is_success && msg.header.gle == Win32Error.ERROR_FILE_NOT_FOUND)
            {
                Model.IsEnabled = false;
                Model.IsHooked = false;
                Model.Address = 0;
                Model.NumberOfRequestIntercepted = 0;
            }
            else
            {
                var driver = msg.body.driver;
                Model.IsHooked = true;
                Model.IsEnabled = driver.IsEnabled;
                Model.Address = driver.Address;
                Model.Name = driver.Name;
                Model.NumberOfRequestIntercepted = driver.NumberOfRequestIntercepted;
            }
        }
    }
}
