#include "DriverUtils.hpp"


///
/// @brief This file contains the kernel specific utility functions and classes
///


namespace CFB::Driver::Utils
{
#pragma region KMutex
    void
    KMutex::Init()
    {
        dbg("Creating KMutex");
        ::KeInitializeMutex(&_mutex, 0);
    }

    void
    KMutex::Clean()
    {
    }

    void KMutex::Lock()
    {
        ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
    }

    void KMutex::Unlock()
    {
        if (!::KeReleaseMutex(&_mutex, true))
            ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
    }
#pragma endregion

#pragma region KSpinLock
    void
    KSpinLock::Init()
    {
        dbg("Creating KSpinLock");
        KeInitializeSpinLock(&_SpinLock);
    }

    void
    KSpinLock::Clean()
    {
        dbg("Cleaning up KSpinLock");
    }

    void
    KSpinLock::Lock()
    {
        KeAcquireSpinLock(&_SpinLock, &_OldIrql);
    }

    void
    KSpinLock::Unlock()
    {
        ::KeReleaseSpinLock(&_SpinLock, _OldIrql);
    }
#pragma endregion

#pragma region KQueuedSpinLock
    void
    KQueuedSpinLock::Init()
    {
        dbg("Creating KQueuedSpinLock");
        ::KeInitializeSpinLock(&_SpinLock);

    }

    void
    KQueuedSpinLock::Clean()
    {
        dbg("Cleaning up KQueuedSpinLock");
    }

    void
    KQueuedSpinLock::Lock()
    {
        ::KeAcquireInStackQueuedSpinLock(&_SpinLock, &_LockQueueHandle);
    }


    void
    KQueuedSpinLock::Unlock()
    {
        ::KeReleaseInStackQueuedSpinLock(&_LockQueueHandle);
    }
#pragma endregion

}
