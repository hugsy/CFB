using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Fuzzer
{
    public class FuzzingRuntimeException : Exception
    {
        public FuzzingRuntimeException() { }

        public FuzzingRuntimeException(string message) : base(message) { }

        public FuzzingRuntimeException(string message, Exception inner) : base(message, inner) { }
    }


    public class FuzzingSession
    {
        private FuzzingStrategy Strategy;
        private Irp Irp;
        private BackgroundWorker Worker;
        private DoWorkEventArgs WorkEvent;
        private string DeviceName;


        public void Start(string DeviceName, FuzzingStrategy Strategy, Irp Irp, BackgroundWorker worker, DoWorkEventArgs evt, int FuzzStartIndex, int FuzzEndIndex)
        {
            this.DeviceName = DeviceName;
            this.Strategy = Strategy;
            this.Irp = Irp;
            this.Worker = worker;
            this.WorkEvent = evt;

            this.Strategy.IndexStart = FuzzStartIndex;
            this.Strategy.IndexEnd = FuzzEndIndex;

            Strategy.Data = Utils.CloneByteArray(Irp.Body);

            if (this.DeviceName.Length == 0)
            {
                this.DeviceName = this.Irp.DeviceName.Replace("\\Device\\", "\\\\.\\");
            }

            Start();
        }


        private void Start()
        {
            uint IoctlCode = this.Irp.Header.IoctlCode;
            byte[] OutputData = new byte[this.Irp.Header.OutputBufferLength];

            Strategy.ContinueGeneratingCases = true;

            foreach (byte[] FuzzedInputData in Strategy.GenerateTestCases())
            {
                //MessageBox.Show($"{BitConverter.ToString(FuzzedInputData)}");

                if (Worker.CancellationPending)
                {
                    Strategy.ContinueGeneratingCases = false;
                    WorkEvent.Cancel = true;
                    break;
                }

                try
                {
                    if (SendFuzzedData(this.DeviceName, IoctlCode, FuzzedInputData, OutputData) == false)
                    {
                        Strategy.ContinueGeneratingCases = false;
                    }
                }
                catch(FuzzingRuntimeException /* Excpt */)
                {
                    Strategy.ContinueGeneratingCases = false;
                    WorkEvent.Cancel = true;
                }
            }
        }


        private bool SendFuzzedData(string DeviceName, uint IoctlCode, byte[] InputData, byte[] OutputData)
        {
            IntPtr hDriver = Kernel32.CreateFile(
                DeviceName,
                Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                0,
                IntPtr.Zero,
                Kernel32.OPEN_EXISTING,
                0,
                IntPtr.Zero
                );

            if (hDriver.ToInt32() == Kernel32.INVALID_HANDLE_VALUE)
            {
                var text = $"Cannot open device '{DeviceName}': {Kernel32.GetLastError().ToString("x8")}";
                throw new FuzzingRuntimeException(text);
            }

            IntPtr lpInBuffer = Marshal.AllocHGlobal(InputData.Length);
            Marshal.Copy(InputData, 0, lpInBuffer, InputData.Length);

            IntPtr pdwBytesReturned = Marshal.AllocHGlobal(sizeof(int));

            IntPtr lpOutBuffer = IntPtr.Zero;
            int dwOutBufferLen = 0;

            
            if (OutputData.Length > 0)
            {
                dwOutBufferLen = OutputData.Length;
                lpOutBuffer = Marshal.AllocHGlobal(dwOutBufferLen);
                // todo : add some checks after the devioctl for some memleaks
            }
            
            bool res = Kernel32.DeviceIoControl(
                hDriver,
                IoctlCode,
                lpInBuffer,
                (uint)InputData.Length,
                lpOutBuffer,
                (uint)dwOutBufferLen,
                pdwBytesReturned,
                IntPtr.Zero
            );


            if(res)
            {
                int dwBytesReturned = (int)Marshal.PtrToStructure(pdwBytesReturned, typeof(int));

                if (OutputData.Length > 0 && dwBytesReturned > 0)
                {
                    if (dwBytesReturned < OutputData.Length)
                    {
                        Marshal.Copy(lpOutBuffer, OutputData, 0, OutputData.Length);
                    }
                    // TODO: signal possible overflow
                }

            }              


            Marshal.FreeHGlobal(pdwBytesReturned);
            Marshal.FreeHGlobal(lpInBuffer);

            if (dwOutBufferLen > 0)
            {
                Marshal.FreeHGlobal(lpOutBuffer);
            }
            
            Kernel32.CloseHandle(hDriver);

            return true;
        }

    }
}