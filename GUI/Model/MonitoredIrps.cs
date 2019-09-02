using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Model
{
    public class MonitoredIrps
    {
        private List<Irp> IrpList;

        public MonitoredIrps()
        {
            IrpList = new List<Irp>();
        }

        public List<Irp> GetIrps()
        {
            return this.IrpList;
        }
    }
}
