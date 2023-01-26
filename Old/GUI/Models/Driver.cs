using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Models
{
    public class Driver : IEquatable<Driver>
    {
        public string Name { get; set; } = "";
        public ulong Address { get; set; } = 0;
        public ulong NumberOfRequestIntercepted { get; set; } = 0;
        public bool IsEnabled { get; set; } = false;
        public bool IsHooked { get; set; } = false;


        public Driver(String name) => Name = name;
        public Driver() : this("") { }


        public bool Equals(Driver other) =>
            Address == other.Address &&
            NumberOfRequestIntercepted == other.NumberOfRequestIntercepted &&
            Name == other.Name &&
            IsEnabled == other.IsEnabled &&
            IsHooked == other.IsHooked;
    }
}
