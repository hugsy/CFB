#include "DriverUtils.hpp"

#ifndef USHRT_MAX
#define USHRT_MAX ((1 << (sizeof(u16) * 8)) - 1)
#endif // USHRT_MAX


///
/// @brief Basic allocator/deallocator for the kernel
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
    case CMCI_LEVEL:
        return "CMCI_LEVEL";
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

#pragma region KUnicodeString

KUnicodeString::KUnicodeString(const wchar_t* src, const u16 srcsz, const POOL_TYPE type)
{
    m_len          = min((srcsz >> 1), USHRT_MAX - 2) + 1;
    const usize sz = size();
    m_buffer       = KAlloc<wchar_t*>(sz, CFB_DEVICE_TAG, type);
    ::RtlCopyMemory(m_buffer.get(), src, sz - sizeof(wchar_t));
    ::RtlInitUnicodeString(&m_str, m_buffer.get());
    dbg("KUnicodeString::KUnicodeString('%wZ', %d, %d)", &m_str, size(), length());
}

KUnicodeString::KUnicodeString(const wchar_t* src, const POOL_TYPE type)
{
    const usize len = ::wcslen(src);
    const u16 sz    = min((len << 1), USHRT_MAX - 2);
    KUnicodeString::KUnicodeString(src, sz, type);
}

KUnicodeString::KUnicodeString(const PUNICODE_STRING src, const POOL_TYPE type)
{
    KUnicodeString::KUnicodeString(src->Buffer, src->Length, type);
}

///
/// @brief The total size allocated by the buffer holding the string
///
/// @return const usize
///
const usize
KUnicodeString::size() const
{
    return length() * sizeof(wchar_t);
}

///
/// @brief The length of the string
///
/// @return const usize
///
const usize
KUnicodeString::length() const
{
    return m_len;
}

KUnicodeString::~KUnicodeString()
{
    dbg("KUnicodeString::~KUnicodeString(%p)", m_str);
}

const PUNICODE_STRING
KUnicodeString::get()
{
    return &m_str;
}

#pragma endregion

#pragma region KMutex
KMutex::KMutex()
{
    // dbg("Creating KMutex");
    ::KeInitializeMutex(&_mutex, 0);
}

KMutex::~KMutex()
{
    // dbg("Destroying KMutex");
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
