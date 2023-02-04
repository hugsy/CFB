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

    ///
    ///@brief Unique smart pointer to the native `DRIVER_OBJECT` associated to the current hooked driver object.
    ///
    Utils::UniquePointer<DRIVER_OBJECT> HookedDriverObject;

    ///
    ///@brief Construct a new Hooked Driver object
    ///
    ///@param UnicodePath the unicode name of the driver
    ///
    HookedDriver(const PUNICODE_STRING UnicodePath);

    ///
    ///@brief Destroy the Hooked Driver object
    ///
    ~HookedDriver();

    ///
    ///@brief HookedDriver memory allocator
    ///
    ///@param sz
    ///@return void*
    ///
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

    ///
    ///@brief HookedDriver memory destructor
    ///
    ///@param Memory
    ///
    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating HookedDriver at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }

    ///
    ///@brief Is the current driver capturing IRP?
    ///
    ///@return true
    ///@return false
    ///
    bool
    CanCapture();

    ///
    ///@brief Enable the IRP capture mode for the driver
    ///
    ///@return true
    ///@return false
    ///
    bool
    EnableCapturing();

    ///
    ///@brief Disable the IRP capture mode for the driver
    ///
    ///@return true
    ///@return false
    ///
    bool
    DisableCapturing();

    ///
    ///@brief Get the number of IRP currently in the stack
    ///
    ///@return usize const
    ///
    usize const
    IrpCount() const;

    ///
    ///@brief Increment the stacked IRP count
    ///
    void
    IncrementIrpCount();

    ///
    ///@brief Alias to `IncrementIrpCount()`
    ///
    ///@return HookedDriver&
    ///
    HookedDriver&
    operator++();

    ///
    ///@brief Decrement the stacked IRP count
    ///
    void
    DecrementIrpCount();

    ///
    ///@brief Alias to `DecrementIrpCount()`
    ///
    ///@return HookedDriver&
    ///
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
    bool m_Enabled;

    ///
    /// @brief The total number of intercepted IRPs
    ///
    u64 m_InterceptedIrpsCount;

    ///
    /// @brief CallbackLock to guard callback access
    ///
    Utils::KFastMutex m_CallbackLock;

    ///
    /// @brief
    ///
    HookState m_State;
};
} // namespace CFB::Driver
