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

        public IEnumerable<Driver> Get()
        {
            return _drivers ?? new List<Driver>();
        }


        /// <summary>
        /// Get all drivers
        /// </summary>
        /// <param name="forceRefresh"></param>
        /// <returns></returns>
        public async Task<IEnumerable<Driver>> GetAsync(bool forceRefresh=false)
        {
            if (_drivers == null || forceRefresh)
                _drivers = await App.BrokerSession.EnumerateDrivers();

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

        public async Task<IEnumerable<Driver>> GetAsync(bool onlyHooked, bool onlyEnabled)
        {
            return await Task.Run(() =>
            {
                return _drivers
                     .Where(
                        x => (onlyHooked && x.IsHooked == true) ||
                            (onlyEnabled && x.IsEnabled == true)
                     );
            });
        }

        public int Count(bool onlyHooked, bool onlyEnabled)
        {
            if (_drivers == null) 
                return 0;

            if (onlyHooked || onlyEnabled)
                return _drivers
                    .Where(
                        x => (onlyHooked && x.IsHooked == true) || 
                            (onlyEnabled && x.IsEnabled == true)
                    ).Count();
            else
                return _drivers.Count();
        }

    }
}
