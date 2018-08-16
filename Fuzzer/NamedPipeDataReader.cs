using System;
using System.IO;
using System.IO.Pipes;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.InteropServices;


namespace Fuzzer
{
    class NamedPipeDataReader
    {
        /// <summary>
        /// This structure mimics the structure SNIFFED_DATA_HEADER from the driver IrpDumper (IrpDumper\PipeComm.h)
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct NamedPipeMessageHeader
        {
            public ulong TimeStamp;
            public char Irql;
            public ulong IoctlCode;
            public ulong Pid;
            public ulong SessionId;
            public ulong BufferLength;
        }


        /// <summary>
        /// Constructor
        /// </summary>
        public NamedPipeDataReader()
        {
            StartClientThread();
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        private void StartClientThread()
        {
            var task = Task.Factory.StartNew(ReadFromPipe);
            task.Wait();
        }


        /// <summary>
        /// Read a message from the CFB named pipe. This function converts the raw bytes into a proper structure.
        /// </summary>
        /// <param name="pipe">The handle to the named pipe</param>
        /// <returns>A tuple of NamedPipeMessageHeader for the header and an array of byte for the body.</returns>
        public Tuple<NamedPipeMessageHeader, byte[]> ReadMessage(NamedPipeClientStream pipe)
        {
            //
            // Read the header first (fixed-length)
            //

            var HeaderSize = Marshal.SizeOf(typeof(NamedPipeMessageHeader));
            var RawHeader = new byte[HeaderSize];   

            pipe.Read(RawHeader, 0, HeaderSize);

            IntPtr ptr = Marshal.AllocHGlobal(HeaderSize);
            Marshal.StructureToPtr(RawHeader, ptr, false);
            NamedPipeMessageHeader Header = (NamedPipeMessageHeader)Marshal.PtrToStructure(ptr, typeof(NamedPipeMessageHeader));

        
            //
            // Read body
            //
            var Body = new byte[Header.BufferLength];
            pipe.Read(Body, 0, Convert.ToInt32(Header.BufferLength));

            return Tuple.Create(Header, Body);
        }


        /// <summary>
        /// Threaded function that'll open a handle to named pipe, and pop out structured messages
        /// </summary>
        void ReadFromPipe()
        {
            using (var CfbPipe = new NamedPipeClientStream(".", "CFB", PipeDirection.In, PipeOptions.Asynchronous) )
            {
                CfbPipe.Connect(0);
                Debug.WriteLine("Connected to named pipe");

                try
                {
                    while (true)
                    {
                        var Message = ReadMessage(CfbPipe);
                        var Header = Message.Item1;
                        var Body = Message.Item2;

                        var line = String.Format("New Ioctl {0:x}, {1} B of data", Header.IoctlCode, Header.BufferLength);
                        Debug.WriteLine(line);
                    }
                    
                }
                catch (Exception Ex)
                {
                    Debug.WriteLine(Ex.Message);
                }

                CfbPipe.Flush();
                Debug.WriteLine("Closing handle");
            }
        }


    }

}
