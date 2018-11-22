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
                if( queue.Count == 1 )
                {
                    // wake up any blocked dequeue
                    Monitor.PulseAll(queue);
                }
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
                if( queue.Count == maxSize - 1 )
                {
                    // wake up any blocked enqueue
                    Monitor.PulseAll(queue);
                }
                return item;
            }
        }

    }
}