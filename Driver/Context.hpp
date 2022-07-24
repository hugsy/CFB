#pragma once

#include "CapturedIrp.hpp"
#include "Collector.hpp"
#include "Common.hpp"
#include "DriverUtils.hpp"
#include "HookedDriverManager.hpp"
#include "Log.hpp"


namespace Utils = CFB::Driver::Utils;

struct GlobalContext
{
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
    /// @brief
    ///
    Utils::KQueuedSpinLock OwnerSpinLock;

    ///
    /// @brief
    ///
    ULONG SessionId;

    ///
    /// @brief Manages the hooked drivers
    ///
    CFB::Driver::HookedDriverManager DriverManager;

    ///
    /// @brief Where all the intercepted IRPs are stored
    ///
    CFB::Driver::DataCollector<CFB::Driver::CapturedIrp> IrpCollector;


    GlobalContext() : DriverObject(nullptr), DeviceObject(nullptr), Owner(nullptr), OwnerSpinLock(), SessionId(-1)
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
        dbg("Allocating GlobalContext");
        void* Memory = ::ExAllocatePoolWithTag(PagedPool, sz, CFB_DEVICE_TAG);
        ::RtlSecureZeroMemory(Memory, sz);
        return Memory;
    }

    static void
    operator delete(void* m)
    {
        dbg("Deallocating GlobalContext");
        return ::ExFreePoolWithTag(m, CFB_DEVICE_TAG);
    }
};

extern struct GlobalContext* Globals;
