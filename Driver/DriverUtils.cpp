#include "DriverUtils.hpp"


///
/// @brief This file contains the kernel specific utility functions and classes
///


namespace CFB::Driver::Utils
{
#pragma region KMutex
    KMutex::KMutex()
    {
        ::KeInitializeMutex(&_mutex, 0);
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
    KSpinLock::KSpinLock()
    {
        KeInitializeSpinLock(&_SpinLock);
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
    KQueuedSpinLock::KQueuedSpinLock()
    {
        dbg("Creating KQueuedSpinLock");
        ::KeInitializeSpinLock(&_SpinLock);
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
