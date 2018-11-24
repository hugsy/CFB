using System.Collections.Generic;
using System.Threading;

namespace Fuzzer
{
    class BlockingQueue<T>
    {
        private readonly Queue<T> queue = new Queue<T>();
        private readonly int maxSize;
        public BlockingQueue(int maxSize)
        {
            this.maxSize = maxSize;
        }

        public void Enqueue(T item)
        {
            lock( queue )
            {
                while( queue.Count >= maxSize )
                {
                    Monitor.Wait(queue);
                }
                queue.Enqueue(item);
                Monitor.PulseAll(queue);
            }
        }

        public T Dequeue()
        {
            lock( queue )
            {
                while( queue.Count == 0 )
                {
                    Monitor.Wait(queue);
                }
                T item = queue.Dequeue();
                Monitor.PulseAll(queue);
                return item;
            }
        }
    }
}