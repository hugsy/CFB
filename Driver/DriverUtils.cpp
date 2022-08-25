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

#pragma region KUnicodeString

KUnicodeString::KUnicodeString(const wchar_t* src, const u16 srclen, const POOL_TYPE type)
{
    dbg("KUnicodeString::KUnicodeString('%S')", src);
    m_len    = min(srclen, USHRT_MAX - 2) + 1;
    m_buffer = KAlloc<wchar_t*>(size(), CFB_DEVICE_TAG, type);
    ::RtlCopyMemory(m_buffer.get(), src, size());
    ::RtlInitUnicodeString(&m_str, m_buffer.get());
}

KUnicodeString::KUnicodeString(const wchar_t* src, const POOL_TYPE type)
{
    const u16 sz = min(::wcslen(src), USHRT_MAX - 2) + 1;
    KUnicodeString::KUnicodeString(src, sz, type);
}

KUnicodeString::KUnicodeString(const PUNICODE_STRING src, const POOL_TYPE type)
{
    KUnicodeString::KUnicodeString(src->Buffer, src->Length, type);
}

const usize
KUnicodeString::size() const
{
    return length() * sizeof(wchar_t);
}

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
    dbg("Creating KMutex");
    ::KeInitializeMutex(&_mutex, 0);
}

KMutex::~KMutex()
{
    dbg("Destroying KMutex");
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
