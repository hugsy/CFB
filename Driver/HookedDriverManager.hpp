#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"
#include "HookedDriver.hpp"
#include "Log.hpp"

#define CFB_MAX_HOOKED_DRIVERS 32

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
struct HookedDriverManager
{
    ///
    /// @brief A pointer to the head of hooked drivers
    ///
    Utils::LinkedList<HookedDriver> Entries;

    ///
    /// @brief A mutex to protect access to the critical resources
    ///
    Utils::KQueuedSpinLock Mutex;

    ///
    /// @brief Construct a new Hooked Driver Manager object
    ///
    HookedDriverManager();

    ///
    /// @brief Destroy the Hooked Driver Manager object
    ///
    ~HookedDriverManager();

    static void*
    operator new(usize sz)
    {
        dbg("Allocating HookedDriverManager");
        return ::ExAllocatePoolWithTag(PagedPool, sz, CFB_DEVICE_TAG);
    }

    static void
    operator delete(void* m)
    {
        dbg("Deallocating HookedDriverManager");
        return ::ExFreePoolWithTag(m, CFB_DEVICE_TAG);
    }

    ///
    /// @brief
    ///
    /// @param UnicodePath
    /// @return NTSTATUS
    ///
    NTSTATUS
    InsertDriver(PUNICODE_STRING const UnicodePath);

    ///
    /// @brief
    ///
    /// @param Path
    /// @return NTSTATUS
    ///
    NTSTATUS
    InsertDriver(const wchar_t* Path);

    ///
    /// @brief
    ///
    /// @param UnicodePath
    /// @return NTSTATUS
    ///
    NTSTATUS
    RemoveDriver(PUNICODE_STRING const UnicodePath);

    ///
    /// @brief
    ///
    /// @param Path
    /// @return NTSTATUS
    ///
    NTSTATUS
    RemoveDriver(const wchar_t* Path);

    ///
    /// @brief
    ///
    /// @return NTSTATUS
    ///
    NTSTATUS
    RemoveAllDrivers();

    ///
    /// @brief  Set the monitoring state for the driver given in argument. If `true`, monitoring will become active,
    /// effectively capturing all the IRPs to the driver. Setting to `false` disables the monitoring.
    ///
    /// @param Path Path to the driver (as a wide string)
    /// @param bEnable `true` to enable, `false` to disable
    /// @return NTSTATUS the error code returned from the function.
    ///
    NTSTATUS
    SetMonitoringState(const wchar_t* Path, bool bEnable);

    ///
    /// @brief Set the monitoring state for the driver given in argument. If `true`, monitoring will become active,
    /// effectively capturing all the IRPs to the driver. Setting to `false` disables the monitoring.
    ///
    /// @param UnicodePath Path to the driver (as a Unicode string)
    /// @param bEnable `true` to enable, `false` to disable
    /// @return NTSTATUS the error code returned from the function.
    ///
    NTSTATUS
    SetMonitoringState(const PUNICODE_STRING UnicodePath, bool bEnable);
};

} // namespace CFB::Driver