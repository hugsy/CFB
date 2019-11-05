using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Models
{
    public class DbObject
    {
        public Guid Id { get; set; } = Guid.NewGuid();
    }
}
