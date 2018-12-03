using System;
using System.Diagnostics;
using System.Linq;

namespace Fuzzer
{
    /// <summary>
    /// 
    /// Bunch of generic static functions 
    /// 
    /// </summary>
    class Utils
    {

        /// <summary>
        /// Get a process name from its PID
        /// </summary>
        /// <param name="ProcessId"></param>
        /// <returns></returns>
        public static string GetProcessById(uint ProcessId)
        {
            string Res = "";

            try
            {
                Process p = Process.GetProcessById(( int )ProcessId);
                Res = p.ProcessName;
            }
            catch
            {
                Res = "";
            }

            return Res;
        }


        /// <summary>
        /// From https://www.codeproject.com/Articles/36747/Quick-and-Dirty-HexDump-of-a-Byte-Array
        /// An hexdump function
        /// </summary>
        /// <param name="InputBytes"></param>
        /// <param name="BytesPerLine"></param>
        /// <returns></returns>
        public static string Hexdump(byte[] InputBytes, int BytesPerLine = 16)
        {
            if( InputBytes == null )
            {
                return "";
            }

            char[] HexCharset = "0123456789ABCDEF".ToCharArray();

            int firstHexColumn =
                  8                   // 8 characters for the address
                + 3;                  // 3 spaces

            int firstCharColumn = firstHexColumn
                + BytesPerLine * 3       // - 2 digit for the hexadecimal value and 1 space
                + ( BytesPerLine - 1 ) / 8 // - 1 extra space every 8 characters from the 9th
                + 2;                  // 2 spaces 

            int lineLength = firstCharColumn
                + BytesPerLine                // - characters to show the ascii value
                + Environment.NewLine.Length; // Carriage return and line feed (should normally be 2)

            char[] line = ( new String(' ', lineLength - Environment.NewLine.Length) + Environment.NewLine ).ToCharArray();
            int expectedLines = ( InputBytes.Length + BytesPerLine - 1 ) / BytesPerLine;
            System.Text.StringBuilder result = new System.Text.StringBuilder(expectedLines * lineLength);

            for( int i = 0 ; i < InputBytes.Length ; i += BytesPerLine )
            {
                line[0] = HexCharset[( i >> 28 ) & 0xF];
                line[1] = HexCharset[( i >> 24 ) & 0xF];
                line[2] = HexCharset[( i >> 20 ) & 0xF];
                line[3] = HexCharset[( i >> 16 ) & 0xF];
                line[4] = HexCharset[( i >> 12 ) & 0xF];
                line[5] = HexCharset[( i >> 8 ) & 0xF];
                line[6] = HexCharset[( i >> 4 ) & 0xF];
                line[7] = HexCharset[( i >> 0 ) & 0xF];

                int hexColumn = firstHexColumn;
                int charColumn = firstCharColumn;

                for( int j = 0 ; j < BytesPerLine ; j++ )
                {
                    if( j > 0 && ( j & 7 ) == 0 )
                        hexColumn++;

                    if( i + j >= InputBytes.Length )
                    {
                        line[hexColumn] = ' ';
                        line[hexColumn + 1] = ' ';
                        line[charColumn] = ' ';
                    }
                    else
                    {
                        byte b = InputBytes[i + j];
                        line[hexColumn] = HexCharset[( b >> 4 ) & 0xF];
                        line[hexColumn + 1] = HexCharset[b & 0xF];
                        line[charColumn] = ( b < 32 ? '·' : ( char )b );
                    }

                    hexColumn += 3;
                    charColumn++;
                }

                result.Append(line);
            }

            return result.ToString();
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="Src"></param>
        /// <returns></returns>
        public static byte[] CloneByteArray(byte[] Src)
        {
            Byte[] ClonedBuffer = new byte[Src.Length];
            Buffer.BlockCopy(Src, 0, ClonedBuffer, 0, Src.Length);
            return ClonedBuffer;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="Src"></param>
        /// <param name="IndexStart"></param>
        /// <param name="IndexEnd"></param>
        /// <returns></returns>
        public static byte[] SliceByteArray(byte[] Src, int IndexStart, int IndexEnd)
        {
            return Src.ToArray().Skip(IndexStart).Take(IndexEnd - IndexStart).ToArray();
        }


        public static byte[] ConvertHexStringToByteArray(string HexString)
        {
            return Enumerable.Range(0, HexString.Length)
                 .Where(x => x % 2 == 0)
                 .Select(x => Convert.ToByte(HexString.Substring(x, 2), 16))
                 .ToArray();
        }
    }
}
