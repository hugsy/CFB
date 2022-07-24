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
    enum class HookState
    {
        Unhooked,
        Hooked
    };

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
    /// @brief A pointer to the driver object. The object refcount has been incremented by the constructor.
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

    ///
    /// @brief
    ///
    HookState State;

    HookedDriver(const PUNICODE_STRING UnicodePath);

    ~HookedDriver();

    static void*
    operator new(const usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(NonPagedPoolNx, sz, CFB_DEVICE_TAG);
        ::RtlSecureZeroMemory(Memory, sz);
        dbg("Allocating HookedDriver at %p", Memory);
        return Memory;
    }

    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating HookedDriver at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }

    void
    SwapCallbacks();


    void
    RestoreCallbacks();
};
} // namespace CFB::Driver
