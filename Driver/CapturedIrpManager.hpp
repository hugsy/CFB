#pragma once

#include "CapturedIrp.hpp"
#include "Common.hpp"
#include "DriverUtils.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
class CapturedIrpManager
{
public:
    ///
    /// @brief Construct a new Data Collector object
    ///
    CapturedIrpManager();

    ///
    /// @brief Destroy the Data Collector object
    ///
    ~CapturedIrpManager();

    ///
    /// @brief Get a reference to the linked list items stored in this container
    ///
    /// @return Utils::LinkedList<CapturedIrp>&
    ///
    Utils::LinkedList<CapturedIrp>&
    Items();

    ///
    /// @brief Set the Event object
    ///
    /// @param hEvent An Event handle
    ///
    /// @return NTSTATUS
    ///
    NTSTATUS
    SetEvent(const HANDLE hEvent);

    ///
    /// @brief Push data to the back of the queue
    ///
    /// @return true
    /// @return false
    ///
    bool
    Push(CapturedIrp* Item);

    ///
    /// @brief Pop the front of the queue
    ///
    /// @return true
    /// @return false
    ///
    CapturedIrp*
    Pop();

    ///
    /// @brief Flush the data in the container
    ///
    void
    Clear();

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
    Utils::LinkedList<CapturedIrp> m_Entries;

    ///
    /// @brief
    ///
    Utils::KFastMutex m_Mutex;
};

} // namespace CFB::Driver
