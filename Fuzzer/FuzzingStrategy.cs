using System;
using System.Collections;
using System.Linq;
using System.Threading;

namespace Fuzzer
{
    abstract public class FuzzingStrategy : IEnumerable
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

        public virtual IEnumerator GetEnumerator()
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


        public override IEnumerator GetEnumerator()
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
