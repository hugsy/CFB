using GUI.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Repositories
{
    //
    // this class contains all the object drivers fetched from the broker
    //
    public class DriverRepository : IAsyncDriverRepository
    {
        private List<Driver> _drivers = null;

        /// <summary>
        /// Get all drivers
        /// </summary>
        /// <param name="forceRefresh"></param>
        /// <returns></returns>
        public async Task<IEnumerable<Driver>> GetAsync(bool forceRefresh=false)
        {
            if (_drivers == null || forceRefresh)
            {
                //
                // Fetch the complete list of driver from the broker. This 
                // operation can be pretty long (1 or 2 ioctls / driver) so we 
                // avoid running it often, only on first time or if user explicitly
                // asks for a driver list refresh
                //
                _drivers = await App.BrokerSession.EnumerateDrivers();
            }

            return _drivers;
        }


        /// <summary>
        /// Get all the drivers whose name matches the given patterns
        /// </summary>
        /// <param name="pattern"></param>
        /// <returns></returns>
        public async Task<IEnumerable<Driver>> GetAsync(string pattern)
        {
            string[] parameters = pattern.Split(' ');
            return await Task.Run(() =>
            {
                return _drivers
                  .Where(driver =>
                      parameters.Any(
                          parameter =>
                              driver.Name.Contains(parameter)
                      )
                  ).OrderByDescending(
                      driver =>
                          parameters.Count(parameter =>
                              driver.Name.Contains(parameter)
                          )
                  );
            });
        }

    }
}
