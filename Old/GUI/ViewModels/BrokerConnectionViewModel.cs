using GUI.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.ViewModels
{
    class BrokerConnectionViewModel : INotifyPropertyChanged
    {
        private ConnectionManager Connection;

        public BrokerConnectionViewModel()
        {
            this.PropertyChanged += new PropertyChangedEventHandler(OnConnectionPropertyChanged);
            Connection = new ConnectionManager();

        }

        public BrokerConnectionStatus Status
        {
            get => Connection.Status;
        }


        private void OnConnectionPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "Status")
            {
                
            }
        }


        #region INotifyPropertyChanged
        private void OnPropertyChanged(string propName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propName));
            }
        }
        public event PropertyChangedEventHandler PropertyChanged;
        #endregion
    }

}
