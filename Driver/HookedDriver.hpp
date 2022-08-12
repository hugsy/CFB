#pragma once

#include "Callbacks.hpp"
#include "Common.hpp"
#include "DriverUtils.hpp"
#include "Log.hpp"

namespace Utils = CFB::Driver::Utils;


namespace CFB::Driver
{
class HookedDriver
{
public:
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
    /// @brief The absolute path of to the driver underneath
    ///
    Utils::KUnicodeString Path;

    ///
    /// @brief A pointer to the driver object. The object refcount has been incremented by the constructor.
    /// The destructor makes sure to release it.
    ///
    PDRIVER_OBJECT OriginalDriverObject;

    Utils::UniquePointer<DRIVER_OBJECT> HookedDriverObject;

    HookedDriver(const PUNICODE_STRING UnicodePath);

    ~HookedDriver();

    static void*
    operator new(const usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(NonPagedPoolNx, sz, CFB_DEVICE_TAG);
        if ( Memory )
        {
            ::RtlSecureZeroMemory(Memory, sz);
            dbg("Allocated HookedDriver at %p", Memory);
        }
        return Memory;
    }

    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating HookedDriver at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }

    bool
    CanCapture();

    bool
    EnableCapturing();

    bool
    DisableCapturing();

    usize const
    IrpCount() const;

    void
    IncrementIrpCount();

    void
    DecrementIrpCount();

    HookedDriver&
    operator++();

    HookedDriver&
    operator--();


private:
    ///
    /// @brief Swap the callbacks of the driver with the interception routines of IrpMonitor
    /// The code of those routines is located in `Driver/Callbacks.cpp`. The function will
    /// also set the object state as `Hooked`.
    ///
    void
    SwapCallbacks();

    ///
    /// @brief Restore the original callbacks and set the object state as `Unhooked`.
    ///
    void
    RestoreCallbacks();

    ///
    /// @brief If `true`, any IRP targetting the driver object underneath will be pushed to the queue of
    /// intercepted IRPs
    ///
    bool Enabled;

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
};
} // namespace CFB::Driver
