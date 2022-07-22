#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
template<typename T>
struct DataCollector
{
    ///
    /// @brief Set when new data is pushed
    ///
    PKEVENT Event;

    ///
    /// @brief Item count
    ///
    usize Count;

    ///
    /// @brief
    ///
    Utils::LinkedList<T> Data;

    ///
    /// @brief
    ///
    Utils::KFastMutex Mutex;

    ///
    /// @brief Construct a new Data Collector object
    ///
    DataCollector() : Event(nullptr), Count(0), Data(), Mutex()
    {
    }

    ///
    /// @brief Destroy the Data Collector object
    ///
    ~DataCollector()
    {
        if ( Event )
        {
            //
            // Free the Event object
            //
            ::KeResetEvent(Event);
            ObDereferenceObject(Event);
        }
    }

    ///
    /// @brief Set the Event object
    ///
    /// @param hEvent
    /// @return true
    /// @return false
    ///
    bool
    SetEvent(HANDLE hEvent)
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

        if ( Event )
        {
            //
            // Exchange the current value, if it was associated to an existing Event, free the reference
            //
            PKEVENT pOldEvent = InterlockedExchangePointer((PVOID*)&Event, pKernelNotifEvent);
            if ( pOldEvent )
            {
                ObDereferenceObject(pOldEvent);
            }
        }

        return true;
    }

    ///
    /// @brief Push data to the back of the queue
    ///
    /// @return true
    /// @return false
    ///
    bool
    Push(T* Item)
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
        if ( Event )
        {
            ::KeSetEvent(Event, 2, false);
        }

        return false;
    }

    ///
    /// @brief Pop the front of the queue
    ///
    /// @return true
    /// @return false
    ///
    T*
    Pop()
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
        if ( Count == 0 && Event )
        {
            ::KeClearEvent(Event);
        }

        return Item;
    }

    ///
    /// @brief Flush the data in the container
    ///
    void
    Clear()
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
};

} // namespace CFB::Driver
