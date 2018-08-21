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
    class CfbDataReader
    {
        public DataTable Messages;
        private Task thread;
        private bool doLoop;
        private Form1 form;

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
            public ulong ProcessId;
            public ulong ThreadId;
            public ulong SessionId;
            public ulong BufferLength;
        }


        /// <summary>
        /// Constructor
        /// </summary>
        public CfbDataReader(Form1 f)
        {

            Messages = new DataTable("IrpData");
            Messages.Columns.Add("TimeStamp", typeof(DateTime));
            Messages.Columns.Add("Irql", typeof(byte));
            Messages.Columns.Add("IoctlCode", typeof(ulong));
            Messages.Columns.Add("ProcessId", typeof(ulong));
            Messages.Columns.Add("ThreadId", typeof(ulong));
            Messages.Columns.Add("SessionId", typeof(ulong));
            Messages.Columns.Add("Buffer", typeof(string));

            form = f;
            doLoop = false;
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        public void StartClientThread()
        {
            form.Log("Starting NamedPipeDataReader thread...");
            thread = Task.Factory.StartNew(ReadFromPipe);
            doLoop = true;
            form.Log("NamedPipeDataReader thread started!");
        }

        /// <summary>
        ///
        /// </summary>
        public void EndClientThread()
        {

            if (thread != null)
            {
                form.Log("Ending NamedPipeDataReader thread...");

                doLoop = false;
                var success_wait = false;

                for (int i = 0; i < 5; i++)
                {
                    form.Log(String.Format("Attempt {0}", i));
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
                    form.Log("Failed to kill gracefully, forcing thread termination!");
                    // TODO
                }

                form.Log("NamedPipeDataReader thread ended!");
            }
        }


        /// <summary>
        /// Read a message from the CFB named pipe. This function converts the raw bytes into a proper structure.
        /// </summary>
        /// <returns>A tuple of NamedPipeMessageHeader for the header and an array of byte for the body.</returns>
        public Tuple<NamedPipeMessageHeader, byte[]> ReadMessage()
        {
            //
            // Read the header first (fixed-length)
            //

            int HeaderSize = Core.MessageHeaderSize();
            IntPtr BufferHeader = Marshal.AllocHGlobal(HeaderSize);
            var Result = Core.ReadMessage(BufferHeader, HeaderSize, new IntPtr(0));
            //Marshal.StructureToPtr(RawHeader, ptr, false);
            NamedPipeMessageHeader Header = (NamedPipeMessageHeader)Marshal.PtrToStructure(BufferHeader, typeof(NamedPipeMessageHeader));


            //
            // Read body
            //
            int BufLen = (int)Header.BufferLength;
            IntPtr BufferBody = Marshal.AllocHGlobal(BufLen);
            Result = Core.ReadMessage(BufferBody, BufLen, new IntPtr(0));
            byte[] Body = (byte[])Marshal.PtrToStructure(BufferBody, typeof(byte[]));
            return Tuple.Create(Header, Body);
        }


        /// <summary>
        /// Threaded function that'll open a handle to named pipe, and pop out structured messages
        /// </summary>
        void ReadFromPipe()
        {

            try
            {
                while (doLoop)
                {
                    var Message = ReadMessage();
                    var Header = Message.Item1;
                    var Body = Message.Item2;

                    // Debug
                    var line = String.Format("Read Ioctl {0:x} from PID {1:d} (TID={2:d}), {3:d} bytes of data",
                            Header.IoctlCode, Header.ProcessId, Header.ThreadId, Header.BufferLength);
                    form.Log(line);
                    // EndofDebug

                    Messages.Rows.Add(
                        DateTime.FromFileTime((long)Header.TimeStamp),
                        Header.Irql,
                        Header.IoctlCode,
                        Header.ProcessId,
                        Header.ThreadId,
                        Header.SessionId,
                        Body
                        );

                }

            }
            catch (Exception Ex)
            {
                form.Log(Ex.Message);
            }

        }
    }
}
