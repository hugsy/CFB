#pragma once

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
    /// @brief
    ///
    CFB::Driver::HookedDriverManager DriverManager;


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
        return ::ExAllocatePoolWithTag(PagedPool, sz, CFB_DEVICE_TAG);
    }

    static void
    operator delete(void* m)
    {
        dbg("Deallocating GlobalContext");
        return ::ExFreePoolWithTag(m, CFB_DEVICE_TAG);
    }
};

extern struct GlobalContext* Globals;
