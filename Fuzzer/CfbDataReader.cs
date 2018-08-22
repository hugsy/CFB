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
        private Form1 RootForm;

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
            public UInt32 IoctlCode;
            public UInt32 ProcessId;
            public UInt32 ThreadId;
            //public UInt32 SessionId;
            public ulong BufferLength;
        }


        /// <summary>
        /// Constructor
        /// </summary>
        public CfbDataReader(Form1 f)
        {

            Messages = new DataTable("IrpData");
            Messages.Columns.Add("TimeStamp", typeof(DateTime));
            Messages.Columns.Add("Irql", typeof(char));
            Messages.Columns.Add("IoctlCode", typeof(ulong));
            Messages.Columns.Add("ProcessId", typeof(ulong));
            Messages.Columns.Add("ThreadId", typeof(ulong));
            //Messages.Columns.Add("SessionId", typeof(ulong));
            Messages.Columns.Add("Buffer", typeof(string));

            RootForm = f;
            doLoop = false;
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        public void StartClientThread()
        {
            RootForm.Log("Starting NamedPipeDataReader thread...");
            thread = Task.Factory.StartNew(ReadFromPipe);
            doLoop = true;
            RootForm.Log("NamedPipeDataReader thread started!");
        }

        /// <summary>
        ///
        /// </summary>
        public void EndClientThread()
        {

            if (thread != null)
            {
                RootForm.Log("Ending NamedPipeDataReader thread...");

                doLoop = false;
                var success_wait = false;

                for (int i = 0; i < 5; i++)
                {
                    RootForm.Log(String.Format("Attempt {0}", i));
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
                    RootForm.Log("Failed to kill gracefully, forcing thread termination!");
                    // TODO
                }

                RootForm.Log("NamedPipeDataReader thread ended!");
            }
        }


        /// <summary>
        /// Read a message from the CFB named pipe. This function converts the raw bytes into a proper structure.
        /// </summary>
        /// <returns>A tuple of NamedPipeMessageHeader for the header and an array of byte for the body.</returns>
        public unsafe Tuple<NamedPipeMessageHeader, byte[]> ReadMessage()
        {
            //
            // Read the header first (fixed-length)
            //
            int HeaderSize = Core.MessageHeaderSize();
            var BufferHeader = Marshal.AllocCoTaskMem(HeaderSize);
            RootForm.Log(String.Format("ReadMessage() - MessageHeaderSize= {0:d}", HeaderSize));
            var Header  = (NamedPipeMessageHeader*)BufferHeader.ToPointer();
            var Result = Core.ReadMessage(Header, HeaderSize);
            NamedPipeMessageHeader Header2 = (NamedPipeMessageHeader)Marshal.PtrToStructure(BufferHeader, typeof(NamedPipeMessageHeader));
            RootForm.Log(String.Format("ReadMessage() - Header= {0:d}", HeaderSize));
            var line = String.Format("Read Ioctl {0:x} from PID {1:lu} (TID={2:lu}), {3:lu} bytes of data",
                            Header2.IoctlCode, Header2.ProcessId, Header2.ThreadId, Header2.BufferLength);
            RootForm.Log(line);

            //
            // Read body
            //
            var BufLen = (int)(*Header).BufferLength;
            var BufferBody = Marshal.AllocCoTaskMem(BufLen);
            var Body  = (byte*)BufferHeader.ToPointer();
            Result = Core.ReadMessage(Body, BufLen);
            byte[] body2 = new byte[BufLen];
            for (int i = 0; i < BufLen; i++)
                body2[i] = Body[i];
 
            Tuple<NamedPipeMessageHeader, byte[]> t = Tuple.Create(Header2, body2);

            Marshal.FreeCoTaskMem(BufferHeader);
            Marshal.FreeCoTaskMem(BufferBody);

            return t;
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

                    Messages.Rows.Add(
                        DateTime.FromFileTime((long)Header.TimeStamp),
                        Header.Irql,
                        Header.IoctlCode,
                        Header.ProcessId,
                        Header.ThreadId,
                        Body
                        );

                }

            }
            catch (Exception Ex)
            {
                RootForm.Log(Ex.Message + "\n" + Ex.StackTrace);
            }

        }
    }
}
