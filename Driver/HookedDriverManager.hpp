#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"
#include "HookedDriver.hpp"
#include "Log.hpp"

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
    /// @brief A Spin Lock to protect access to the critical resources
    ///
    Utils::KSpinLock SpinLock;


    HookedDriverManager();

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

    NTSTATUS
    InsertDriver(const wchar_t* Path);

    NTSTATUS
    RemoveDriver(const wchar_t* Path);
};

} // namespace CFB::Driver
