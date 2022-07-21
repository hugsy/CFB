#include "Collector.hpp"

namespace CFB::Driver
{

template<typename T>
bool
DataCollector<T>::SetEvent(HANDLE hEvent)
{
    //
    // Get a reference to the Event object
    //
    PKEVENT pKernelNotifEvent = nullptr;
    NTSTATUS Status =
        ::ObReferenceObjectByHandle(hEvent, EVENT_ALL_ACCESS, *ExEventObjectType, UserMode, &Event, nullptr);
    if ( !NT_SUCCESS(Status) )
    {
        return false;
    }

    //
    // Exchange the current value, if it was associated to an existing Event, free the reference
    //
    PKEVENT pOldEvent = InterlockedExchangePointer((PVOID*)&Event, pKernelNotifEvent);
    if ( pOldEvent )
    {
        ObDereferenceObject(pOldEvent);
    }

    return true;
}

template<typename T>
bool
DataCollector<T>::Push(T* Item)
{
    Utils::ScopedLock lock(Mutex);

    //
    // Insert the item in the queue
    //
    Data.Insert(Item);
    Count++;

    //
    // Set the event to notify the broker some data is ready
    //
    ::KeSetEvent(Event, 2, false);

    return false;
}


template<typename T>
T*
DataCollector<T>::Pop()
{
    Utils::ScopedLock lock(Mutex);

    //
    // Stored data are treated as a FIFO queue
    //
    T* Item = Data.PopTail();
    if ( Item == nullptr )
    {
        return nullptr;
    }

    Count--;

    //
    // Unset the event is no data is ready
    //
    if ( Count == 0 )
    {
        ::KeClearEvent(&Event);
    }

    return Item;
}

template<typename T>
void
DataCollector<T>::Clear()
{
    do
    {
        auto Item = Pop();
        if ( !Item )
        {
            break;
        }
    } while ( true );
}


} // namespace CFB::Driver
