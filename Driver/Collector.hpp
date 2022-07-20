#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{

struct CapturedIrp
{
};

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

    DataCollector()
    {
    }

    ~DataCollector()
    {
    }

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
};

} // namespace CFB::Driver
