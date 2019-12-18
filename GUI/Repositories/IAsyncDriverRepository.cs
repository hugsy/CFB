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
        Task<IEnumerable<Driver>> GetAsync(bool forceRefresh);


        /// <summary>
        /// Returns all IRPs matching a field matching the given pattern. 
        /// </summary>
        Task<IEnumerable<Driver>> GetAsync(string pattern);

    }
}

