using System;
using System.IO;
using System.IO.Pipes;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Data;

namespace Fuzzer
{
    class NamedPipeDataReader
    {
        public DataTable Messages;
        private Task thread;
        private bool doLoop;

        public bool IsThreadRunning
        {
            get
            {
                return !doLoop;
            }
        }

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
            
            Messages = new DataTable("IrpData");
            Messages.Columns.Add("TimeStamp", typeof(DateTime));
            Messages.Columns.Add("Irql", typeof(byte));
            Messages.Columns.Add("IoctlCode", typeof(ulong));
            Messages.Columns.Add("ProcessId", typeof(ulong));
            Messages.Columns.Add("SessionId", typeof(ulong));
            Messages.Columns.Add("Buffer", typeof(byte[]));

            doLoop = false;
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        public void StartClientThread()
        {
            Debug.WriteLine("Starting NamedPipeDataReader thread...");
            thread = Task.Factory.StartNew(ReadFromPipe);
            doLoop = true;
            Debug.WriteLine("NamedPipeDataReader thread started!");
        }

        /// <summary>
        /// 
        /// </summary>
        public void EndClientThread()
        {
            Debug.WriteLine("Ending NamedPipeDataReader thread...");
            doLoop = false;
            var success_wait = false;

            for(int i=0; i<5; i++)
            {
                Debug.WriteLine( String.Format("Attempt {0}", i) );
                Int32 waitFor = 1*1000; // 1 second
                if (!thread.Wait(waitFor))
                {
                    continue;
                }
                success_wait = true;
                break;
            }

            if (!success_wait)
            {
                Debug.WriteLine("Failed to kill gracefully, forcing thread termination!");

            }
            
            Debug.WriteLine("NamedPipeDataReader thread ended!");
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
                Debug.WriteLine("Waiting to connect to pipe");
                CfbPipe.Connect();
                Debug.WriteLine("Connected to named pipe");

                try
                {
                    while (doLoop)
                    {
                        var Message = ReadMessage(CfbPipe);
                        var Header = Message.Item1;
                        var Body = Message.Item2;

                        // Debug 
                        var line = String.Format("Read Ioctl {0:x} from PID {1:d}, {2:d} bytes of data", 
                            Header.IoctlCode, Header.Pid, Header.BufferLength);
                        Debug.WriteLine(line);
                        // EndofDebug

                        Messages.Rows.Add(
                            DateTime.FromFileTime((long)Header.TimeStamp),
                            Header.Irql,
                            Header.IoctlCode,
                            Header.Pid,
                            Header.SessionId,
                            Body
                            );
                        
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
