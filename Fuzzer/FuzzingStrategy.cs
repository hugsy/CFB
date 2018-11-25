using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace Fuzzer
{
    public abstract class FuzzingStrategy
    {
        protected string Name;
        public bool ContinueGeneratingCases;
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
            return this.Name;
        }

        public virtual IEnumerable<byte[]> GenerateTestCases()
        {
            throw new NotImplementedException();
        }

    }


    public class RandomFuzzingStrategy : FuzzingStrategy
    {

        private Random Rng;
        

        public RandomFuzzingStrategy()
        {
            Name = "Random";
            Rng = new Random();
        }


        public override IEnumerable<byte[]> GenerateTestCases()
        {
            ContinueGeneratingCases = true;

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
}
