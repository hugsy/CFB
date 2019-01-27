using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace Fuzzer
{
    public abstract class FuzzingStrategy
    {
        protected string name;
        protected string description;
        public volatile bool ContinueGeneratingCases;
        public int ForceDelayBetweenCases;
        public int IndexStart, IndexEnd;
        public byte[] Data;

        public FuzzingStrategy()
        {
            this.IndexStart = 0;
            this.IndexEnd = 0;
            this.Data = null;
            this.ContinueGeneratingCases = false;
            this.ForceDelayBetweenCases = 0;
        }

        public override string ToString()
        {
            return this.name;
        }

        public virtual IEnumerable<byte[]> GenerateTestCases()
        {
            throw new NotImplementedException();
        }

        public string Description
        {
            get
            {
                return this.description;
            }
        }

    }


    public class RandomFuzzingStrategy : FuzzingStrategy
    {

        protected Random Rng;


        public RandomFuzzingStrategy()
        {
            name = "Random";
            description = "Infinitely generates random data (unless Max Test Cases is not empty and higher than 0)";
            Rng = new Random();
        }


        protected virtual void FillBuffer(byte[] buffer)
        {
            Rng.NextBytes(buffer);
        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            while (true)
            {
                if (ContinueGeneratingCases == false)
                    yield break;

                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Byte[] FuzzedBuffer = Utils.SliceByteArray(ClonedBuffer, IndexStart, IndexEnd);
                FillBuffer(FuzzedBuffer);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, IndexStart, FuzzedBuffer.Length);

                yield return ClonedBuffer;

                if (ForceDelayBetweenCases > 0)
                {
                    Thread.Sleep(ForceDelayBetweenCases);
                }
                    
            }
        }
    }

    public class RandomAsciiFuzzingStrategy : RandomFuzzingStrategy
    {
        private readonly byte[] Charset;

        public RandomAsciiFuzzingStrategy()
        {
            name = "ASCII Character Overwrite";
            description = "Generate random ASCII";
            Rng = new Random();
            Charset = System.Text.Encoding.ASCII.GetBytes("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ " ); 
        }

        protected override void FillBuffer(byte[] buffer)
        {
            byte[] b = Enumerable.Repeat(Charset, buffer.Length).Select(s => s[Rng.Next(s.Length)]).ToArray();
            for (int i = 0; i < b.Length; i++)
            {
                buffer[i] = b[i];
            }
        }

    }


    public class RandomUnicodeFuzzingStrategy : RandomFuzzingStrategy
    {
        private readonly byte[] Charset;

        public RandomUnicodeFuzzingStrategy()
        {
            name = "Unicode Character Overwrite";
            description = "Generate random Unicode character";
            Rng = new Random();
            Charset = System.Text.Encoding.ASCII.GetBytes("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ ");
        }

        protected override void FillBuffer(byte[] buffer)
        {
            int FuzzLen;

            if (buffer.Length < 2)
            {
                return;
            }         

            if (buffer.Length % 2 == 0)
            {
                FuzzLen = buffer.Length / 2;
            }
            else
            {
                FuzzLen = (buffer.Length / 2) - 1;
            }

            byte[] b = Enumerable.Repeat(Charset, buffer.Length / 2).Select(s => s[Rng.Next(s.Length)]).ToArray();

            for (int i = 0; i < b.Length; i+=2)
            {
                buffer[i] = b[i];
                buffer[i + 1] = 0x00;
            }
        }

    }



    public class BitflipFuzzingStrategy : FuzzingStrategy
    {
        public BitflipFuzzingStrategy()
        {
            name = "Bitflip";
            description = "Successively fuzz the MSB of every QWORD, DWORD and WORD.";
        }


        private IEnumerable<byte[]> MsbBitFlip(int IndexStart, int StepSize)
        {
            Byte[] ClonedBuffer = Utils.CloneByteArray(Data);

            for (int i = IndexStart; i < ClonedBuffer.Length; i += StepSize)
            {
                byte old = ClonedBuffer[i];
                byte b = ClonedBuffer[i];

                if ((b & 0b10000000) != 0)
                {
                    b &= 0b01111111;
                }
                else
                {
                    b &= 0b01111111;
                }

                ClonedBuffer[i] = b;
                yield return ClonedBuffer;
                ClonedBuffer[i] = old;
            }

            ClonedBuffer = null;
        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            var Step = 0;

            while (ContinueGeneratingCases)
            {

                switch (Step)
                {
                    // msb qword (little endian)
                    case 0:
                            foreach(var v in MsbBitFlip(7, 8))
                                yield return v;
                            Step += 1;
                            break;


                    // msb qword (big endian)
                    case 1:
                            foreach (var v in MsbBitFlip(0, 8))
                                yield return v;
                            Step += 1;
                            break;


                    // msb dword (little endian)
                    case 2:
                            foreach (var v in MsbBitFlip(3, 4))
                                yield return v;
                            Step += 1;
                            break;


                    // msb dword (big endian)
                    case 3:
                            foreach (var v in MsbBitFlip(0, 4))
                                yield return v;
                            Step += 1;
                            break;


                    // msb word (little endian)
                    case 4:
                            foreach (var v in MsbBitFlip(1, 2))
                                yield return v;
                            Step += 1;
                            break;



                    // msb word (big endian)
                    case 5:
                            foreach (var v in MsbBitFlip(0, 2))
                                yield return v;
                            Step += 1;
                            break;

                    default:
                        ContinueGeneratingCases = false;
                        break;
                }
            }
        }
    }


    public class BigintOverwriteFuzzingStrategy : FuzzingStrategy
    {
        public BigintOverwriteFuzzingStrategy()
        {
            name = "BigInt Overwrite";
            description = "Successively overwrite every QWORD, DWORD, WORD and BYTE with large integers.";
        }


        private IEnumerable<byte[]> BigIntegerOverwrite(int StartIndex, int StepSize)
        {
            
            for (int i = StartIndex; i < Data.Length; i += StepSize)
            {
                Byte[] FuzzedBuffer = Utils.SliceByteArray(Data, i, i+StepSize);
                for (int j = 0; j < FuzzedBuffer.Length; j++)
                    FuzzedBuffer[j] = 0xff;
                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, i, FuzzedBuffer.Length);

                yield return ClonedBuffer;
            }

            for (int i = StartIndex; i < Data.Length; i += StepSize)
            {
                Byte[] FuzzedBuffer = Utils.SliceByteArray(Data, i, i + StepSize);
                for (int j = 0; j < FuzzedBuffer.Length; j++)
                    FuzzedBuffer[j] = 0x7f;
                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, i, FuzzedBuffer.Length);

                yield return ClonedBuffer;
            }

            for (int i = StartIndex; i < Data.Length; i += StepSize)
            {
                Byte[] FuzzedBuffer = Utils.SliceByteArray(Data, i, i + StepSize);
                for (int j = 0; j < FuzzedBuffer.Length; j++)
                    FuzzedBuffer[j] = 0x00;
                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, i, FuzzedBuffer.Length);

                yield return ClonedBuffer;
            }

        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            var Step = 0;
            var StartIdx = 0;

            while (ContinueGeneratingCases)
            {
                StartIdx = 0;

                switch (Step)
                {
                    // qword 
                    case 0:
                        while (StartIdx < 8)
                        {
                            foreach (var v in BigIntegerOverwrite(StartIdx, 8))
                                yield return v;
                            StartIdx += 1;
                        }
                        Step += 1;
                        break;


                    // dword
                    case 1:
                        while (StartIdx < 4)
                        {
                            foreach (var v in BigIntegerOverwrite(StartIdx, 4))
                               yield return v;
                            StartIdx += 1;
                        }
                        Step += 1;
                        break;


                    // word
                    case 2:
                        while (StartIdx < 2)
                        {
                            foreach (var v in BigIntegerOverwrite(StartIdx, 2))
                                yield return v;
                            StartIdx += 1;
                        }
                        Step += 1;
                        break;


                    // byte
                    case 3:
                        foreach (var v in BigIntegerOverwrite(0, 1))
                            yield return v;
                        Step += 1;
                        break;


                    default:
                        ContinueGeneratingCases = false;
                        break;
                }
            }
        }
    }



}
