namespace Fuzzer
{
    public class Irp
    {
        public CfbDataReader.CfbMessageHeader Header;
        public string DriverName;
        public string DeviceName;
        public byte[] Body;

        public Irp()
        {
        }

        public override string ToString()
        {
            return $"IRP{{#{DeviceName},IOCTL:#{Header.IoctlCode},PID:#{Header.ProcessId} }}";
        }
    }
}
