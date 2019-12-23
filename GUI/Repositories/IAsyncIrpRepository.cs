using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GUI.Models;


namespace GUI.Repositories
{
    public interface IAsyncIrpRepository
    {
        /// <summary>
        /// Returns all IRPs. 
        /// </summary>
        Task<IEnumerable<Irp>> GetAsync();


        /// <summary>
        /// Returns all IRPs matching a field matching the given pattern. 
        /// </summary>
        Task<IEnumerable<Irp>> GetAsync(string pattern);


        Task<bool> Insert(Irp irp);


        Task<bool> Clear();


        Task<bool> Delete(int id);
    }
}
