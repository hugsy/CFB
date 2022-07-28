#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
template<typename T>
class DataCollector
{
public:
    ///
    /// @brief Construct a new Data Collector object
    ///
    DataCollector() : m_Event(nullptr), m_Count(0), m_Data(), m_Mutex()
    {
    }

    ///
    /// @brief Destroy the Data Collector object
    ///
    ~DataCollector()
    {
        if ( m_Event )
        {
            //
            // Free the Event object
            //
            ::KeResetEvent(m_Event);
            ObDereferenceObject(m_Event);
        }
    }

    ///
    /// @brief Get a reference to the linked list items stored in this container
    ///
    /// @return Utils::LinkedList<T>&
    ///
    Utils::LinkedList<T>&
    Items()
    {
        return m_Data;
    }

    ///
    /// @brief Set the Event object
    ///
    /// @param hEvent An Event handle
    ///
    /// @return NTSTATUS
    ///
    NTSTATUS
    SetEvent(const HANDLE hEvent)
    {
        NTSTATUS Status  = STATUS_UNSUCCESSFUL;
        PKEVENT NewEvent = nullptr;

        auto lock = Utils::ScopedLock(m_Mutex);

        //
        // Get a reference to the Event object
        //
        Status = ::ObReferenceObjectByHandle(
            hEvent,
            EVENT_ALL_ACCESS,
            *ExEventObjectType,
            UserMode,
            (PVOID*)&NewEvent,
            nullptr);
        if ( !NT_SUCCESS(Status) )
        {
            return Status;
        }

        //
        // If an event object was already assigned, replace it
        //
        if ( m_Event != nullptr )
        {
            ObDereferenceObject(m_Event);
            m_Event = nullptr;
        }

        m_Event = NewEvent;

        return Status;
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
        Utils::ScopedLock lock(m_Mutex);

        //
        // Push the item to the back of the queue
        //
        m_Data.PushBack(Item);
        m_Count++;

        //
        // Set the event to notify the broker some data is ready
        //
        if ( m_Event )
        {
            ::KeSetEvent(m_Event, 2, false);
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
        Utils::ScopedLock lock(m_Mutex);

        //
        // Stored data are treated as a FIFO queue
        //
        T* Item = m_Data.PopFront();
        if ( Item == nullptr )
        {
            return nullptr;
        }

        m_Count--;

        //
        // Unset the event is no data is ready
        //
        if ( m_Count == 0 && m_Event )
        {
            ::KeClearEvent(m_Event);
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

private:
    ///
    /// @brief Set when new data is pushed
    ///
    PKEVENT m_Event;

    ///
    /// @brief Item count
    ///
    usize m_Count;

    ///
    /// @brief
    ///
    Utils::LinkedList<T> m_Data;

    ///
    /// @brief
    ///
    Utils::KFastMutex m_Mutex;
};

} // namespace CFB::Driver
