using GUI.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI.ViewModels
{
    public class IrpReplay
    {

        /// <summary>
        /// send the irp to the broker with the given inputbuffer, returns an array of byte of the result
        /// </summary>
        /// <returns></returns>
        public async Task<Tuple<uint, byte[]>> SendIrp(
            string deviceName, 
            int ioctlCode, 
            byte[] inputBuffer, 
            int inputBufferLength,
            int outputBufferLength
        )
        {
            var outputBuffer = await App.BrokerSession.ReplayIrp(
                deviceName,
                ioctlCode,
                inputBuffer,
                inputBufferLength,
                outputBufferLength
            );

            return outputBuffer;
        }

    }
}
