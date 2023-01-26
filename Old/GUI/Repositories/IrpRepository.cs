using GUI.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.Repositories
{
    public class IrpRepository : IAsyncIrpRepository
    {
        private List<Irp> _irps = new List<Irp>();

        public async Task<bool> Clear()
        {
            return await Task.Run(() =>
            {
                _irps.Clear();
                return true;
            });
        }



        public async Task<bool> Delete(int id)
        {
            return await Task.Run(() =>
            {
                try
                {
                    _irps.RemoveAt(id);
                    return true;
                }
                catch
                {
                    return false;
                }

            });
        }



        public async Task<IEnumerable<Irp>> GetAsync()
        {
            return await Task.Run(() =>
            {
                return _irps;
            });
        }



        public async Task<IEnumerable<Irp>> GetAsync(string pattern)
        {
            string[] parameters = pattern.Split(' ');

            return await Task.Run(() =>
            {
            return _irps
              .Where(
                irp => parameters.Any(
                    parameter =>
                        irp.header.DriverName.StartsWith(parameter) ||
                        irp.header.DeviceName.StartsWith(parameter) ||
                        irp.header.ProcessName.StartsWith(parameter)
                    )
                ).OrderByDescending(
                    irp => parameters.Count(
                        parameter =>
                            irp.header.DriverName.StartsWith(parameter) ||
                            irp.header.DeviceName.StartsWith(parameter) ||
                            irp.header.ProcessName.StartsWith(parameter)
                     )
                );
            });
        }


        public async Task<bool> Insert(Irp irp)
        {
            try
            {
                await Task.Run(() =>
                {
                    _irps.Add(irp);
                });

                return true;
            }
            catch
            {
                return false;
            }
        }


        public int Count()
        {
            if (_irps == null) return 0;
            return _irps.Count();
        }
    }
}
