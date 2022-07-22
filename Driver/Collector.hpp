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
    SetEvent(HANDLE hEvent);

    ///
    /// @brief Push data to the back of the queue
    ///
    /// @return true
    /// @return false
    ///
    bool
    Push(T*);

    ///
    /// @brief Pop the front of the queue
    ///
    /// @return true
    /// @return false
    ///
    T*
    Pop();

    ///
    /// @brief Flush the data in the container
    ///
    void
    Clear();
};

} // namespace CFB::Driver
