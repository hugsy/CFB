using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Popups;

namespace GUI.Helpers
{
    public static class Utils
    {
        public static string Base64Encode(string plainText)
        {
            var plainTextBytes = System.Text.Encoding.UTF8.GetBytes(plainText);
            return Base64Encode(plainTextBytes);
        }

        public static string Base64Encode(byte[] plainText)
        {
            return System.Convert.ToBase64String(plainText);
        }

        public static byte[] Base64Decode(string base64EncodedData)
        {
            return System.Convert.FromBase64String(base64EncodedData);
        }

        public static async Task ShowPopUp(string msg, string title = "")
        {
            var dialog = new MessageDialog(msg, title);
            await dialog.ShowAsync();
        }


        private static string HexdumpI(byte[] bytes, int bytesPerLine = 16, bool showOffset = true, bool showAscii = true, int addressOffset = 0)
        {
            if (bytes == null) 
                return "<null>";

            char[] hexCharset = "0123456789ABCDEF".ToCharArray();
            char[] asciiCharset = "................................ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~.".ToCharArray();

            var lines = new List<string>();
            var fieldSeparator = "   ";

            for (var off = 0; off < bytes.Length; off += bytesPerLine)
            {
                var hexAddress = (addressOffset + off).ToString("x8");
                var hexaLine  = new char[3 * (bytesPerLine + 1)];
                var asciiLine = new char[bytesPerLine + 1];
                var line = new StringBuilder();

                for (int i=0, j=0, k=0; i<bytesPerLine; i++, j+=3, k++)
                {

                    if(i == bytesPerLine / 2)
                    {
                        hexaLine[j]   = ' ';
                        hexaLine[j+1] = ' ';
                        hexaLine[j+2] = ' ';
                        j += 3;

                        asciiLine[k] = ' ';
                        k++;
                    }

                    if (off + i >= bytes.Length)
                    {
                        hexaLine[j] = ' ';
                        hexaLine[j + 1] = ' ';
                        hexaLine[j + 2] = ' ';

                        asciiLine[k] = ' ';
                        continue;
                    }

                    var b = bytes[off + i];
                    hexaLine[j] = hexCharset[(b & 0xF0) >> 4];
                    hexaLine[j+1] = hexCharset[(b & 0x0F)];
                    hexaLine[j+2] = ' ';

                    if ((b & 0x80) == 0x80)
                        asciiLine[k] = '.';
                    else
                        asciiLine[k] = asciiCharset[b];
                }

                
                if(showOffset)
                {
                    line.Append(hexAddress);
                    line.Append(fieldSeparator);
                }

                line.Append(hexaLine);

                if (showAscii)
                {
                    line.Append(fieldSeparator);
                    line.Append(asciiLine);
                }

                lines.Add(line.ToString());
            }

            return String.Join(Environment.NewLine, lines.ToArray()); 
        }


        public static string Hexdump(byte[] bytes)
            => HexdumpI(bytes, 16, true, true, 0);


        public static string SimpleHexdump(byte[] bytes)
            => HexdumpI(bytes, 16, false, false, 0);


        public static byte[] StringToByteArray(string hex)
        {
            if (hex.Length % 2 == 1)
                throw new Exception("The binary key cannot have an odd number of digits");

            byte[] arr = new byte[hex.Length >> 1];

            for (int i = 0; i < hex.Length >> 1; ++i)
            {
                arr[i] = (byte)((GetHexVal(hex[i << 1]) << 4) + (GetHexVal(hex[(i << 1) + 1])));
            }

            return arr;
        }


        private static int GetHexVal(char hex)
        {
            int val = (int)hex;
            return val - (val < 58 ? 48 : 55);
        }

        public static string FormatMessage(uint Status)
            => $"{Enum.GetName(typeof(Win32Error), Status)} - 0x{Status:x8}";
    }
}
