#include "DriverUtils.hpp"


///
/// @brief Basic allocator/deallocator for the kernel
///
///
void* __cdecl
operator new(usize Size, POOL_TYPE PoolType, ULONG PoolTag)
{
    void* Memory = ::ExAllocatePoolWithTag(PoolType, Size, PoolTag);
    dbg("new(Size=%d, Type=%d, Tag=%x)=%p", Size, PoolType, PoolTag, Memory);
    return Memory;
}

void __cdecl
operator delete(void* Memory, usize Size)
{
    dbg("delete(Memory=%p, Size=%d)", Memory, Size);
    ::ExFreePoolWithTag(Memory, 0);
}

void* __cdecl
operator new[](size_t Size, POOL_TYPE PoolType)
{
    return operator new(Size, PoolType);
}

void __cdecl
operator delete[](void* Memory, usize Size)
{
    return operator delete(Memory, Size);
}


///
/// @brief This namespace contains the kernel specific utility functions and classes
///


namespace CFB::Driver::Utils
{

#pragma region Logging helper functions
const char*
ToString(KIRQL const Level)
{
    switch ( Level )
    {
    case PASSIVE_LEVEL:
        return "PASSIVE_LEVEL";
    case APC_LEVEL:
        return "APC_LEVEL";
    case DISPATCH_LEVEL:
        return "DISPATCH_LEVEL";
#ifdef CMCI_LEVEL
    case CMCI_LEVEL:
        return "CMCI_LEVEL";
#endif // CMCI_LEVEL
    case CLOCK_LEVEL:
        return "CLOCK_LEVEL";
    case POWER_LEVEL:
        return "POWER_LEVEL";
    case HIGH_LEVEL:
        return "HIGH_LEVEL";
    }
    return "INVALID_LEVEL";
}
#pragma endregion

#pragma region KMutex
KMutex::KMutex()
{
    ::KeInitializeMutex(&_mutex, 0);
}

KMutex::~KMutex()
{
}

void
KMutex::Lock()
{
    ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
}

void
KMutex::Unlock()
{
    if ( !::KeReleaseMutex(&_mutex, true) )
        ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
}
#pragma endregion

#pragma region KFastMutex
KFastMutex::KFastMutex()
{
    ::ExInitializeFastMutex(&_mutex);
}

KFastMutex::~KFastMutex()
{
}

void
KFastMutex::Lock()
{
    ::ExAcquireFastMutex(&_mutex);
}

void
KFastMutex::Unlock()
{
    ::ExReleaseFastMutex(&_mutex);
}

#pragma endregion

#pragma region KCriticalRegion


KCriticalRegion::KCriticalRegion()
{
    ::ExInitializeResourceLite(&_mutex);
}


KCriticalRegion::~KCriticalRegion()
{
    ::ExDeleteResourceLite(&_mutex);
}

void
KCriticalRegion::Lock()
{
    (void)::ExEnterCriticalRegionAndAcquireResourceExclusive(&_mutex);
}


void
KCriticalRegion::Unlock()
{
    ::ExReleaseResourceAndLeaveCriticalRegion(&_mutex);
}


#pragma endregion

#pragma region KSpinLock
KSpinLock::KSpinLock()
{
    KeInitializeSpinLock(&_SpinLock);
}

KSpinLock::~KSpinLock()
{
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
    ::KeInitializeSpinLock(&_SpinLock);
}

KQueuedSpinLock::~KQueuedSpinLock()
{
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


} // namespace CFB::Driver::Utils
