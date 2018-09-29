using System;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Data;
using System.Windows;
using System.Windows.Forms;
using System.Collections.Concurrent;


namespace Fuzzer
{
    public class CfbDataReader
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


        public DataTable Messages;
        private Thread MessageCollectorThread, MessageDisplayThread;
        private BlockingCollection<Irp> NewIrpQueue;
        private bool doLoop;
        private Form1 RootForm;
        public List<Irp> Irps;
        private AutoResetEvent NewMessageEvent;
        private IntPtr NewMessageEventHandler;
        private BindingSource bs;

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
            Messages.Columns.Add("IrqLevel", typeof(string));
            Messages.Columns.Add("IoctlCode", typeof(string));
            Messages.Columns.Add("ProcessId", typeof(UInt32));
            Messages.Columns.Add("ProcessName", typeof(string));
            Messages.Columns.Add("ThreadId", typeof(UInt32));
            Messages.Columns.Add("BufferLength", typeof(UInt32));
            Messages.Columns.Add("DriverName", typeof(string));
            Messages.Columns.Add("DeviceName", typeof(string));
            Messages.Columns.Add("Buffer", typeof(string));

            RootForm = f;

            bs = new BindingSource();
            RootForm.IrpDataView.DataSource = bs;
            bs.DataSource = Messages;
            bs.ResetBindings(false);

            doLoop = false;
            Irps = new List<Irp>();
            NewIrpQueue = new BlockingCollection<Irp>();
            NewMessageEvent = new AutoResetEvent(false);
            NewMessageEventHandler = NewMessageEvent.SafeWaitHandle.DangerousGetHandle();
        }


        /// <summary>
        /// Starts a dedicated thread to pop out messages from the named pipe.
        /// </summary>
        public void StartClientThread()
        {
            //
            // Pass the handler to the C# event to our driver
            // src: http://www.yoda.arachsys.com/csharp/threads/waithandles.shtml
            // 
            
            RootForm.Log($"Sending {NewMessageEventHandler:x} to driver...");
            
            if (Core.SetEventNotificationHandle(NewMessageEventHandler) == false)
            {
                int ErrNo = Marshal.GetLastWin32Error();
                RootForm.Log($"Failed to pass the event handle to the driver, cannot pursue: GetLastError()=0x{ErrNo:x}");
                return;
            }

                       
            MessageDisplayThread = new Thread(DisplayMessagesThreadRoutine)
            {
                Name = "MessageDisplayThread",
                Priority = ThreadPriority.BelowNormal,
                IsBackground = true
            };

            MessageCollectorThread = new Thread(PopMessagesFromDriverThreadRoutine)
            {
                Name = "MessageCollectorThread",
                Priority = ThreadPriority.BelowNormal,
                IsBackground = true
            };


            RootForm.Log("Starting threads...");

            MessageDisplayThread.Start();
            MessageCollectorThread.Start();

            doLoop = true;

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

            if (t == null || !t.IsAlive)
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
        private Irp ReadMessage()
        {
            int HeaderSize;
            int ErrNo;
            bool bResult;
            int dwNumberOfByteRead;
            IntPtr lpdwNumberOfByteRead;

            var handles = new WaitHandle[] { NewMessageEvent, };

            //
            // Read the raw header 
            //
            HeaderSize = Core.GetCfbMessageHeaderSize();


            //
            // Get the exact size of the next message
            //

            lpdwNumberOfByteRead =  Marshal.AllocHGlobal(sizeof(int));


            //
            // Wait for and pop one new message
            //
            while (true)
            {
                WaitHandle.WaitAny(handles);
                NewMessageEvent.Reset();

                bResult = Core.ReadCfbDevice(IntPtr.Zero, 0, lpdwNumberOfByteRead);

                if (!bResult)
                {
                    ErrNo = Marshal.GetLastWin32Error();
                    throw new Exception($"ReadMessage() - ReadCfbDevice(HEADER) while querying message: GetLastError()=0x{ErrNo:x}");
                }

                dwNumberOfByteRead = (int)Marshal.PtrToStructure(lpdwNumberOfByteRead, typeof(int));

                if (dwNumberOfByteRead == 0)
                    continue;

                RootForm.Log($"ReadMessage() - ReadCfbDevice() new message of {dwNumberOfByteRead:d} Bytes");

                if (dwNumberOfByteRead < HeaderSize)
                {
                    throw new Exception($"ReadMessage() - announced size of {dwNumberOfByteRead:x} B is too small");
                }

                break;
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

            char[] charsToTrim = { '\0' };

            // header
            CfbMessageHeader Header = (CfbMessageHeader)Marshal.PtrToStructure(RawMessage, typeof(CfbMessageHeader));

            // driver name
            byte[] DriverNameBytes = new byte[2*0x104];
            Marshal.Copy(RawMessage + Marshal.SizeOf(typeof(CfbMessageHeader)), DriverNameBytes, 0, DriverNameBytes.Length);
            string DriverName = System.Text.Encoding.Unicode.GetString(DriverNameBytes).Trim(charsToTrim);

            // driver name
            byte[] DeviceNameBytes = new byte[2*0x104];
            Marshal.Copy(RawMessage + Marshal.SizeOf(typeof(CfbMessageHeader)) + DriverNameBytes.Length, DeviceNameBytes, 0, DeviceNameBytes.Length);
            string DeviceName = System.Text.Encoding.Unicode.GetString(DeviceNameBytes).Trim(charsToTrim);

            // body          
            byte[] Body = new byte[Header.BufferLength];
            Marshal.Copy(RawMessage + HeaderSize, Body, 0, Convert.ToInt32(Header.BufferLength));


            Marshal.FreeHGlobal(lpdwNumberOfByteRead);
            Marshal.FreeHGlobal(RawMessage);


            RootForm.Log($"Read Ioctl {Header.IoctlCode:x} by '{DriverName:s}' from (PID={Header.ProcessId:d},TID={Header.ThreadId:d}), BodyLen={Header.BufferLength:d}B");

            return new Irp
            {
                Header = Header,
                DriverName = DriverName,
                DeviceName = DeviceName,
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
                RootForm.Log(Ex.Message + "\n" + Ex.StackTrace);
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
                    Irp irp = NewIrpQueue.Take();

                    Messages.Rows.Add(
                        DateTime.FromFileTime((long)irp.Header.TimeStamp),
                        IrlqToHuman(irp.Header.Irql),
                        "0x" + irp.Header.IoctlCode.ToString("x8"),
                        irp.Header.ProcessId,
                        GetProcessById(irp.Header.ProcessId),
                        irp.Header.ThreadId,
                        irp.Header.BufferLength,
                        irp.DriverName,
                        irp.DeviceName,
                        BitConverter.ToString(irp.Body)
                        );

                    RefreshData();

                }
            }
            catch (Exception Ex)
            {
                RootForm.Log(Ex.Message + "\n" + Ex.StackTrace);
            }

        }

        private string IrlqToHuman(byte irql)
        {
            string hexvalue = "0x" + irql.ToString("x2");
            string strvalue = "";

            switch (irql)
            {
                case 0:
                    strvalue = "PASSIVE_LEVEL";
                    break;

                case 1:
                    strvalue = "APC_LEVEL";
                    break;

                case 2:
                    strvalue = "DPC_LEVEL";
                    break;

                default:
                    strvalue = "??";
                    break;   
            }

            return $"{strvalue:s} ({hexvalue:s})" ;
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

        private void RefreshData()
        {
            bs.ResetBindings(false);
            RootForm.IrpDataView.FirstDisplayedScrollingRowIndex = RootForm.IrpDataView.RowCount - 1;
            RootForm.IrpDataView.Refresh();
        }


    }
}