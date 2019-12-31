using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GUI.Models;


namespace GUI.Repositories
{
    public interface IAsyncDriverRepository
    {
        /// <summary>
        /// Returns all drivers. 
        /// </summary>
        Task<IEnumerable<Driver>> GetAsync(bool forceRefresh=false);


        /// <summary>
        /// Returns all drivers whose names match the given pattern. 
        /// </summary>
        Task<IEnumerable<Driver>> GetAsync(string pattern);

        /// <summary>
        /// Returns all hooked drivers 
        /// </summary>
        Task<IEnumerable<Driver>> GetAsync(bool onlyHooked, bool onlyEnabled);


        int Count(bool onlyHooked, bool onlyEnabled);
    }
}

