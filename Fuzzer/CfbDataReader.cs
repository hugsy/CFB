using System;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Data;
using System.Windows.Forms;
using System.Collections.Concurrent;
using System.Text;

namespace Fuzzer
{
    class CfbDataReader
    {
        /// <summary>
        /// This structure mimics the structure SNIFFED_DATA_HEADER from the driver IrpDumper (IrpDumper\PipeComm.h)
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct CfbMessageHeader
        {
            public ulong TimeStamp;
            public byte Irql;
            public byte Padding0;
            public byte Padding1;
            public byte Padding2;
            public UInt32 IoctlCode;
            public UInt32 ProcessId;
            public UInt32 ThreadId;
            public UInt32 BufferLength;
        }


        /// <summary>
        /// 
        /// </summary>
        public struct IRP
        {
            public CfbMessageHeader Header;
            public string DriverName;
            public byte[] Body;
        }


        public DataTable Messages;
        private Thread MessageCollectorThread, MessageDisplayThread;
        private BlockingCollection<IRP> NewIrpQueue;
        private bool doLoop;
        private Form1 RootForm;
        private List<IRP> Irps;


        public bool IsThreadRunning
        {
            get
            {
                return !doLoop;
            }
        }


        /// <summary>
        /// Constructor
        /// </summary>
        public CfbDataReader(Form1 f)
        {
            Messages = new DataTable("IrpData");
            Messages.Columns.Add("TimeStamp", typeof(DateTime));
            Messages.Columns.Add("Irql", typeof(string));
            Messages.Columns.Add("IoctlCode", typeof(string));
            Messages.Columns.Add("ProcessId", typeof(UInt32));
            Messages.Columns.Add("ProcessName", typeof(string));
            Messages.Columns.Add("ThreadId", typeof(UInt32));
            Messages.Columns.Add("BufferLength", typeof(UInt32));
            Messages.Columns.Add("DriverName", typeof(string));
            Messages.Columns.Add("Buffer", typeof(string));

            RootForm = f;
            doLoop = false;
            Irps = new List<IRP>();
            NewIrpQueue = new BlockingCollection<IRP>();
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        public void StartClientThread()
        {
            RootForm.Log("Starting threads...");

            doLoop = true;

            MessageDisplayThread = new Thread(DisplayMessagesThreadRoutine)
            {
                Name = "MessageDisplayThread"
            };

            MessageCollectorThread = new Thread(PopMessagesFromDriverThreadRoutine)
            {
                Name = "MessageCollectorThread"
            };
  
            MessageDisplayThread.Start();
            MessageCollectorThread.Start();
            RootForm.Log("Threads started!");
        }


        /// <summary>
        /// Tries to end cleanly all the threads.
        /// </summary>
        public void EndClientThreads()
        {
            JoinThread(MessageCollectorThread);
            JoinThread(MessageDisplayThread);
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="t"></param>
        private void JoinThread(Thread t)
        {
            doLoop = false;

            if (!t.IsAlive || t==null)
                return;

            RootForm.Log($"Ending thread '{t.Name:s}'...");

            Int32 waitFor = 1*1000; // 1 second

            for (int i = 0; i < 5; i++)
            {
                if (t.Join(waitFor) == false)
                {
                    continue;
                }

                RootForm.Log($"Thread '{t.Name:s}' ended!");
                return;
            }

            RootForm.Log("Failed to kill gracefully, forcing thread termination!");
            try
            {
                MessageCollectorThread.Abort();
            }
            catch (ThreadAbortException TaEx)
            {
                RootForm.Log($"Thread '{t.Name:s}' killed: {TaEx.Message:s}");
            }
        }


        /// <summary>
        /// Read a message from the CFB driver. This function converts the raw bytes into a proper structure.
        /// </summary>
        /// <returns>An IRP struct object for the header and an array of byte for the body.</returns>
        private IRP ReadMessage()
        {
            int HeaderSize;
            int ErrNo;
            bool bResult;
            int dwNumberOfByteRead;
            IntPtr lpdwNumberOfByteRead;

            //
            // Read the raw header 
            //
            HeaderSize = Core.GetCfbMessageHeaderSize();


            //
            // Get the exact size of the next message
            //

            lpdwNumberOfByteRead =  Marshal.AllocHGlobal(sizeof(int));
            
            while(true)
            { 
                bResult = Core.ReadCfbDevice(IntPtr.Zero, 0, lpdwNumberOfByteRead);

                if (!bResult)
                {
                    ErrNo = Marshal.GetLastWin32Error();
                    throw new Exception($"ReadMessage() - ReadCfbDevice(HEADER) while querying message: GetLastError()=0x{ErrNo:x}");
                }

                /*
                ErrNo = Marshal.GetLastWin32Error();
                if (ErrNo != 0x7A) // ERROR_INSUFFICIENT_BUFFER
                {
                    throw new Exception($"ReadMessage() - ReadCfbDevice(HEADER) failed unexpectedly: GetLastError()=0x{ErrNo:x}");
                }
                */

                dwNumberOfByteRead = (int)Marshal.PtrToStructure(lpdwNumberOfByteRead, typeof(int));
                
                if (dwNumberOfByteRead == 0)
                {
                    // replace w/ event
                    System.Threading.Thread.Sleep(5000);
                    continue;
                }

                RootForm.Log($"ReadMessage() - ReadCfbDevice() new message of {dwNumberOfByteRead:d} Bytes");

                break;
            }


            if (dwNumberOfByteRead < HeaderSize)
            {
                throw new Exception($"ReadMessage() - announced size of {dwNumberOfByteRead:x} B is too small");
            }


            //
            // Get the whole thing
            //

            var RawMessage = Marshal.AllocHGlobal(dwNumberOfByteRead);

            bResult = Core.ReadCfbDevice(RawMessage, dwNumberOfByteRead, new IntPtr(0));

            if (!bResult)
            {
                ErrNo = Marshal.GetLastWin32Error();
                throw new Exception($"ReadMessage() - ReadCfbDevice() failed while reading message: GetLastError()=0x{ErrNo:x}");
            }



            //
            // And convert it to managed code
            //

            // header
            CfbMessageHeader Header = (CfbMessageHeader)Marshal.PtrToStructure(RawMessage, typeof(CfbMessageHeader));

            // driver name
            byte[] DriverNameBytes = new byte[2*0x104];
            Marshal.Copy(RawMessage + Marshal.SizeOf(typeof(CfbMessageHeader)), DriverNameBytes, 0, DriverNameBytes.Length);
            char[] charsToTrim = { '\0' };
            string DriverName = System.Text.Encoding.Unicode.GetString(DriverNameBytes).Trim(charsToTrim);
            
            // body          
            byte[] Body = new byte[Header.BufferLength];
            Marshal.Copy(RawMessage + HeaderSize, Body, 0, Convert.ToInt32(Header.BufferLength));


            Marshal.FreeHGlobal(lpdwNumberOfByteRead);
            Marshal.FreeHGlobal(RawMessage);


            RootForm.Log($"Read Ioctl {Header.IoctlCode:x} from driver '{DriverName:s}' to PID={Header.ProcessId:d},TID={Header.ThreadId:d}, {Header.BufferLength:d} bytes of data");

            return new IRP
            {
                Header = Header,
                DriverName = DriverName,
                Body = Body
            };
        }


        /// <summary>
        /// Threaded function that'll open a handle to named pipe, and pop out structured messages
        /// </summary>
        private void PopMessagesFromDriverThreadRoutine()
        {
            RootForm.Log("Starting PopMessagesFromDriverThreadRoutine");

            try
            {
                while (doLoop)
                {
                    var irp = ReadMessage();
                    Irps.Add(irp);

                    int NbItems = Irps.Count;
                    RootForm.Log($"Pushing IRP #{NbItems:d}");
                    NewIrpQueue.Add(irp);
                }
            }
            catch (Exception Ex)
            {
                //RootForm.Log(Ex.Message + "\n" + Ex.StackTrace);
                MessageBox.Show(Ex.Message + "\n" + Ex.StackTrace);
            }
        }


        /// <summary>
        /// Pops new IRPs from the queue and display them in the DataGridView
        /// </summary>
        private void DisplayMessagesThreadRoutine()
        {
            RootForm.Log("Starting DisplayMessagesThreadRoutine");
            try
            {
                while (doLoop)
                {
                    IRP Irp = NewIrpQueue.Take();
                    var Body = "";

                    foreach (byte b in Irp.Body)
                    {
                        Body += $"{b:x2} ";
                    }

                    Messages.Rows.Add(
                        DateTime.FromFileTime((long)Irp.Header.TimeStamp),
                        "0x" + Irp.Header.Irql.ToString("x2"),
                        "0x" + Irp.Header.IoctlCode.ToString("x8"),
                        Irp.Header.ProcessId,
                        GetProcessById(Irp.Header.ProcessId),
                        Irp.Header.ThreadId,
                        Irp.Header.BufferLength,
                        Irp.DriverName,
                        Body
                        );
                }
            }
            catch (Exception Ex)
            {
                RootForm.Log(Ex.Message + "\n" + Ex.StackTrace);
            }

        }


        /// <summary>
        /// Simple wrapper around Process.GetProcessById
        /// </summary>
        /// <param name="ProcessId"></param>
        /// <returns></returns>
        private string GetProcessById(uint ProcessId)
        {
            string Res = "";

            try
            {
                Process p = Process.GetProcessById((int)ProcessId);
                Res = p.ProcessName;
            }
            catch
            {
                Res = "";
            }

            return Res;
        }

    }
}