#define CFB_NS "[CFB::Driver::Utils]"

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

#pragma region KUnicodeString

KUnicodeString::KUnicodeString(const wchar_t* src, const u16 srcsz, const POOL_TYPE type) :
    m_UnicodeString {},
    m_StringBuffer {KAlloc<wchar_t*>(sizeof(wchar_t) + srcsz, CFB_DEVICE_TAG, type)}
{
    m_UnicodeString.Buffer        = m_StringBuffer.get();
    m_UnicodeString.Length        = srcsz;
    m_UnicodeString.MaximumLength = srcsz + sizeof(wchar_t);

    ::memset(m_StringBuffer.get(), 0, capacity());
    ::memcpy(m_StringBuffer.get(), src, srcsz);

    dbg("KUnicodeString::KUnicodeString((wchar_t*)L'%wZ', length=%lluB, capacity=%lluB)",
        &m_UnicodeString,
        size(),
        capacity());
}

KUnicodeString::KUnicodeString(PUNICODE_STRING&& src, const POOL_TYPE type) : m_UnicodeString {*src}, m_StringBuffer {}
{
    dbg("KUnicodeString::KUnicodeString((&&)L'%wZ', length=%lluB, capacity=%lluB)",
        &m_UnicodeString,
        size(),
        capacity());
}


KUnicodeString::KUnicodeString(PUNICODE_STRING const& src, const POOL_TYPE type) :
    m_UnicodeString {},
    m_StringBuffer {KAlloc<wchar_t*>(src->MaximumLength, CFB_DEVICE_TAG, type)}
{
    ::memcpy(m_StringBuffer.get(), src->Buffer, MIN(src->Length, src->MaximumLength));
    m_UnicodeString.Buffer        = m_StringBuffer.get();
    m_UnicodeString.Length        = src->Length;
    m_UnicodeString.MaximumLength = src->MaximumLength;

    dbg("KUnicodeString::KUnicodeString((const&)L'%wZ', length=%lluB, capacity=%lluB)",
        &m_UnicodeString,
        size(),
        capacity());
}


KUnicodeString::~KUnicodeString()
{
    dbg("KUnicodeString::~KUnicodeString(%p)", m_UnicodeString);
}


KUnicodeString::KUnicodeString(const KUnicodeString& other)
{
    m_StringBuffer = KAlloc<wchar_t*>(other.capacity());
    ::memcpy(m_StringBuffer.get(), other.data(), other.size());

    m_UnicodeString.Buffer        = m_StringBuffer.get();
    m_UnicodeString.Length        = other.size();
    m_UnicodeString.MaximumLength = other.capacity();
}

KUnicodeString&
KUnicodeString::operator=(const KUnicodeString& other) noexcept
{
    if ( this != &other )
    {
        m_StringBuffer = KAlloc<wchar_t*>(other.capacity());
        ::memcpy(m_StringBuffer.get(), other.get(), other.size());

        m_UnicodeString.Buffer        = m_StringBuffer.get();
        m_UnicodeString.Length        = other.size();
        m_UnicodeString.MaximumLength = other.capacity();
    }
    return *this;
}


KUnicodeString&
KUnicodeString::operator=(KUnicodeString&& other) noexcept
{
    if ( this != &other )
    {
        m_StringBuffer = static_cast<KAlloc<wchar_t*>&&>(other.m_StringBuffer);
        ::memcpy(&m_UnicodeString, other.get(), sizeof(UNICODE_STRING));
        ::memset(&other.m_UnicodeString, 0, sizeof(UNICODE_STRING));
    }
    return *this;
}


bool
KUnicodeString::operator==(KUnicodeString const& other)
{
    return size() == other.size() && ::RtlCompareUnicodeString(get(), other.get(), true) == 0;
}


bool
KUnicodeString::operator==(PUNICODE_STRING const& other)
{
    return size() == other->Length && ::RtlCompareUnicodeString(get(), other, true) == 0;
}


const wchar_t*
KUnicodeString::data() const
{
    return m_UnicodeString.Buffer;
}


const PUNICODE_STRING
KUnicodeString::get() const
{
    return const_cast<PUNICODE_STRING>(&m_UnicodeString);
}


const usize
KUnicodeString::size() const
{
    return m_UnicodeString.Length;
}


const usize
KUnicodeString::capacity() const
{
    return m_UnicodeString.MaximumLength;
}

#pragma endregion KUnicodeString

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
