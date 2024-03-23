#pragma once

// clang-format off
#include "Common.hpp"
#include "DriverUtils.hpp"
#include "Log.hpp"

#include "CapturedIrpManager.hpp"
#include "HookedDriverManager.hpp"
// clang-format on


#define CFB_MAX_HEXDUMP_BYTE 64

namespace Driver = CFB::Driver;
namespace Utils  = CFB::Driver::Utils;

struct GlobalContext
{
    ///
    /// @brief Any critical read/write operation to the global context structure must acquire this lock.
    ///
    Utils::KQueuedSpinLock ContextLock;

    ///
    /// @brief A pointer to the device object
    ///
    PDRIVER_OBJECT DriverObject;

    ///
    /// @brief A pointer to the device object
    ///
    PDEVICE_OBJECT DeviceObject;

    ///
    /// @brief A pointer to the EPROCESS of the broker. Not more than one handle to the
    /// device is allowed.
    ///
    PEPROCESS Owner;

    ///
    /// @brief Incremental session ID number.
    ///
    ULONG SessionId;

    ///
    /// @brief Manages the hooked drivers
    ///
    Driver::HookedDriverManager DriverManager;

    ///
    /// @brief Where all the intercepted IRPs are stored
    ///
    Driver::CapturedIrpManager IrpManager;


    GlobalContext() : DriverObject {nullptr}, DeviceObject {nullptr}, Owner {nullptr}, ContextLock {}, SessionId(-1)
    {
        dbg("Creating GlobalContext");
    }


    ~GlobalContext()
    {
        dbg("Destroying GlobalContext");
        DriverObject = nullptr;
        DeviceObject = nullptr;
        Owner        = nullptr;
    }

    static void*
    operator new(usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(NonPagedPoolNx, sz, CFB_DEVICE_TAG);
        if ( Memory )
        {
            dbg("Allocated GlobalContext at %p", Memory);
            ::RtlSecureZeroMemory(Memory, sz);
        }
        return Memory;
    }

    static void
    operator delete(void* m)
    {
        dbg("Deallocating GlobalContext");
        ::ExFreePoolWithTag(m, CFB_DEVICE_TAG);
        m = nullptr;
        return;
    }
};

///
/// @brief Reference to the global driver context.
///
extern struct GlobalContext* Globals;
