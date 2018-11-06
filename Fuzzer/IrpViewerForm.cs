using System;
using System.Text;
using System.Windows.Forms;


namespace Fuzzer
{
    public partial class IrpViewerForm : Form
    {
        private Irp Irp;
        private int Index;

        public IrpViewerForm(int Index, Irp irp)
        {
            InitializeComponent();
            this.Text = $"HexViewer for IRP #{Index:d} (IoctlNumber={irp.Header.IoctlCode:x}) to {irp.DeviceName:s}";
            this.Irp = irp;
            this.Index = Index;

            UpdateIrpDetailFields();
            UpdateIrpBodyTextBox();
        }

        private void UpdateIrpDetailFields()
        {
            IrpIndexTextBox.Text = this.Index.ToString();
            IrpDriverNameTextBox.Text = this.Irp.DriverName;
            IrpDeviceNameTextBox.Text = this.Irp.DeviceName;
            IrpTimestampTextBox.Text = DateTime.FromFileTime((long)this.Irp.Header.TimeStamp).ToString();
            IrpProcessNameTextBox.Text = CfbDataReader.GetProcessById(this.Irp.Header.ProcessId) + $" ({this.Irp.Header.ProcessId})";
            IrpIoctlCodeTextBox.Text = $"0x{this.Irp.Header.IoctlCode:x8}";
            IrpIrqlTextBox.Text = CfbDataReader.IrqlAsString(this.Irp.Header.Irql) + $" (0x{this.Irp.Header.Irql})";
        }

        private void UpdateIrpBodyTextBox()
        {
            IrpBodyHexdumpTextBox.Text = "" +  Hexdump(this.Irp.Body);
        }

        // https://www.codeproject.com/Articles/36747/Quick-and-Dirty-HexDump-of-a-Byte-Array
        private static string Hexdump(byte[] InputBytes, int BytesPerLine = 16 )
        {
            if (InputBytes == null)
            {
                return ""; 
            }

            char[] HexCharset = "0123456789ABCDEF".ToCharArray();

            int firstHexColumn =
                  8                   // 8 characters for the address
                + 3;                  // 3 spaces

            int firstCharColumn = firstHexColumn
                + BytesPerLine * 3       // - 2 digit for the hexadecimal value and 1 space
                + (BytesPerLine - 1) / 8 // - 1 extra space every 8 characters from the 9th
                + 2;                  // 2 spaces 

            int lineLength = firstCharColumn
                + BytesPerLine                // - characters to show the ascii value
                + Environment.NewLine.Length; // Carriage return and line feed (should normally be 2)

            char[] line = (new String(' ', lineLength - Environment.NewLine.Length) + Environment.NewLine).ToCharArray();
            int expectedLines = (InputBytes.Length + BytesPerLine - 1) / BytesPerLine;
            StringBuilder result = new StringBuilder(expectedLines * lineLength);

            for (int i = 0; i < InputBytes.Length; i += BytesPerLine)
            {
                line[0] = HexCharset[(i >> 28) & 0xF];
                line[1] = HexCharset[(i >> 24) & 0xF];
                line[2] = HexCharset[(i >> 20) & 0xF];
                line[3] = HexCharset[(i >> 16) & 0xF];
                line[4] = HexCharset[(i >> 12) & 0xF];
                line[5] = HexCharset[(i >> 8) & 0xF];
                line[6] = HexCharset[(i >> 4) & 0xF];
                line[7] = HexCharset[(i >> 0) & 0xF];

                int hexColumn = firstHexColumn;
                int charColumn = firstCharColumn;

                for (int j = 0; j < BytesPerLine; j++)
                {
                    if (j > 0 && (j & 7) == 0)
                        hexColumn++;

                    if (i + j >= InputBytes.Length)
                    {
                        line[hexColumn] = ' ';
                        line[hexColumn + 1] = ' ';
                        line[charColumn] = ' ';
                    }
                    else
                    {
                        byte b = InputBytes[i + j];
                        line[hexColumn] = HexCharset[(b >> 4) & 0xF];
                        line[hexColumn + 1] = HexCharset[b & 0xF];
                        line[charColumn] = (b < 32 ? '·' : (char)b);
                    }

                    hexColumn += 3;
                    charColumn++;
                }

                result.Append(line);
            }

            return result.ToString(); 
        }



 
    }
}
