#pragma once

#include "Callbacks.hpp"
#include "Common.hpp"
#include "DriverUtils.hpp"
#include "Log.hpp"

namespace Utils = CFB::Driver::Utils;


namespace CFB::Driver
{
struct HookedDriver
{
    ///
    /// @brief The next HookedDriver entry (if any)
    ///
    LIST_ENTRY Next;

    ///
    /// @brief If `true`, any IRP targetting the driver object underneath will be pushed to the queue of
    /// intercepted IRPs
    ///
    bool Enabled;

    ///
    /// @brief The absolute path of to the driver underneath
    ///
    UNICODE_STRING Path;

    ///
    /// @brief A pointer to the driver object. The object refcount has been incremented by the manager.
    /// The destructor makes sure to release it.
    ///
    PDRIVER_OBJECT DriverObject;

    ///
    /// @brief The array where the original `IRP_MJ_*` callbacks are stored
    ///
    PDRIVER_DISPATCH OriginalRoutines[IRP_MJ_MAXIMUM_FUNCTION];

    ///
    /// @brief A pointer to where the original `FastIoRead` callback is stored
    ///
    PFAST_IO_READ FastIoRead;

    ///
    /// @brief A pointer to where the original `FastIoWrite` callback is stored
    ///
    PFAST_IO_WRITE FastIoWrite;

    ///
    /// @brief A pointer to where the original `FastIoDeviceControl` callback is stored
    ///
    PFAST_IO_DEVICE_CONTROL FastIoDeviceControl;

    ///
    /// @brief The total number of intercepted IRPs
    ///
    u64 InterceptedIrpsCount;

    ///
    /// @brief Mutex to guard callback access
    ///
    Utils::KFastMutex Mutex;

    HookedDriver(const wchar_t* _Path, const PDRIVER_OBJECT _DriverObject);

    ~HookedDriver();

    static void*
    operator new(usize sz)
    {
        dbg("Allocating HookedDriver");
        void* Memory = ::ExAllocatePoolWithTag(PagedPool, sz, CFB_DEVICE_TAG);
        ::RtlSecureZeroMemory(Memory, sz);
        return Memory;
    }

    static void
    operator delete(void* m)
    {
        dbg("Deallocating HookedDriver");
        return ::ExFreePoolWithTag(m, CFB_DEVICE_TAG);
    }

    void
    SwapCallbacks();
};
} // namespace CFB::Driver
