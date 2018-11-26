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

        private Random Rng;


        public RandomFuzzingStrategy()
        {
            name = "Random";
            description = "Infinitely generates random data (unless Max Test Cases is not empty and higher than 0)";
            Rng = new Random();
        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            while (true)
            {
                if (ContinueGeneratingCases == false)
                    yield break;

                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Byte[] FuzzedBuffer = Utils.SliceByteArray(ClonedBuffer, IndexStart, IndexEnd);
                Rng.NextBytes(FuzzedBuffer);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, IndexStart, FuzzedBuffer.Length);

                yield return ClonedBuffer;

                if (ForceDelayBetweenCases > 0)
                    Thread.Sleep(ForceDelayBetweenCases);
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


        private IEnumerable<byte[]> BigIntegerOverwrite(int StepSize)
        {
            
            for (int i = 0; i < Data.Length; i += StepSize)
            {
                Byte[] FuzzedBuffer = Utils.SliceByteArray(Data, i, i+StepSize);
                for (int j = 0; j < FuzzedBuffer.Length; j++)
                    FuzzedBuffer[j] = 0xff;
                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, i, FuzzedBuffer.Length);

                yield return ClonedBuffer;
            }

            for (int i = 0; i < Data.Length; i += StepSize)
            {
                Byte[] FuzzedBuffer = Utils.SliceByteArray(Data, i, i + StepSize);
                for (int j = 0; j < FuzzedBuffer.Length; j++)
                    FuzzedBuffer[j] = 0xff;
                FuzzedBuffer[FuzzedBuffer.Length - 1] = 0x7f;
                Byte[] ClonedBuffer = Utils.CloneByteArray(Data);
                Buffer.BlockCopy(FuzzedBuffer, 0, ClonedBuffer, i, FuzzedBuffer.Length);

                yield return ClonedBuffer;
            }

        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            var Step = 0;

            while (ContinueGeneratingCases)
            {
                switch (Step)
                {
                    // qword 
                    case 0:
                        foreach (var v in BigIntegerOverwrite(8))
                            yield return v;
                        Step += 1;
                        break;


                    // dword
                    case 1:
                        foreach (var v in BigIntegerOverwrite(4))
                            yield return v;
                        Step += 1;
                        break;


                    // word
                    case 2:
                        foreach (var v in BigIntegerOverwrite(2))
                            yield return v;
                        Step += 1;
                        break;


                    // byte
                    case 3:
                        foreach (var v in BigIntegerOverwrite(1))
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
