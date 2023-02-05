#pragma once

#include "Common.hpp"
#include "Log.hpp"
#include "Utils.hpp"


void* __cdecl
operator new(size_t Size, POOL_TYPE PoolType = NonPagedPoolNx, ULONG PoolTag = CFB_DEVICE_TAG);

void __cdecl
operator delete(void* Memory, usize Size);

void* __cdecl
operator new[](size_t Size, POOL_TYPE PoolType);

void __cdecl
operator delete[](void* Memory, usize Size);

template<typename T>
T&
move(T& other)
{
    return static_cast<T&&>(other);
}

namespace CFB::Driver::Utils
{

//////
///@brief
///
///@param Level
///@return const char*
///
///
/// @param Level
/// @return const char*
///
const char*
ToString(KIRQL const Level);

///
/// @brief Typed class for locking/unlocking a specific type to a scope
///
/// @tparam T
///
template<typename T>
class ScopedLock
{
public:
    ScopedLock(T& lock) : _lock(lock)
    {
        _lock.Lock();
    }

    ~ScopedLock()
    {
        _lock.Unlock();
    }

    ScopedLock(const ScopedLock&) = delete;

    ScopedLock(const ScopedLock&&) = delete;

    ScopedLock&
    operator=(const ScopedLock& other) = delete;

    ScopedLock&
    operator=(const ScopedLock&& other) = delete;

private:
    T& _lock;
};


///
/// @brief
///
///
class ScopedIrql
{
public:
    ScopedIrql(KIRQL level) : m_NewIrql(level)
    {
        KeRaiseIrql(m_NewIrql, &m_OldIrql);
        dbg("IRQL %s -> %s", ToString(m_OldIrql), ToString(m_NewIrql));
    }

    ~ScopedIrql()
    {
        ::KeLowerIrql(m_OldIrql);
        dbg("IRQL %d <- %d", ToString(m_OldIrql), ToString(m_NewIrql));
    }

private:
    KIRQL m_OldIrql;
    KIRQL m_NewIrql;
};


///
/// @brief
///
/// @tparam T
/// @tparam D
///
template<typename T, typename D>
class ScopedWrapper
{
public:
    ScopedWrapper(T& f, D d) : _f(f), _d(d)
    {
    }

    ~ScopedWrapper()
    {
        _d();
    }

    T
    get() const
    {
        return _f;
    }

private:
    T& _f;
    D _d;
};


///
/// @brief Generic allocator in the kernel
///
/// @tparam T
///
template<typename T>
class KAlloc
{
public:
    KAlloc(const usize sz = 0, const u32 tag = CFB_DEVICE_TAG, POOL_TYPE type = NonPagedPoolNx) :
        m_PoolType(type),
        m_PoolTag(tag),
        m_Size(sz),
        m_Buffer(nullptr)
    {
        if ( sz )
        {
            allocate(CFB::Utils::Memory::AlignValue(sz, 0x10));
        }
    }

    ~KAlloc()
    {
        if ( valid() )
        {
            free();
        }
    }

    //
    // No copy/move constructor or copy assignment
    //
    KAlloc(const KAlloc&) = delete;
    KAlloc(KAlloc&&)      = delete;
    KAlloc&
    operator=(const KAlloc& other) noexcept = delete;

    //
    // Move assignment ok to allow:
    // KAlloc a;
    // [...]
    // a = KAlloc(...)
    //
    KAlloc&
    operator=(KAlloc&& other) noexcept
    {
        if ( this != &other )
        {
            // if allocated, free first
            free();

            m_Buffer  = other.m_Buffer;
            m_PoolTag = other.m_PoolTag;
            m_Size    = other.m_Size;

            other.m_Size    = 0;
            other.m_PoolTag = 0;
            other.m_Buffer  = nullptr;
        }
        return *this;
    }

    T
    operator[](usize idx)
    {
        return m_Buffer[idx];
    }

    const T
    get() const
    {
        return m_Buffer;
    }

    const usize
    size() const
    {
        return m_Size;
    }

    bool
    valid() const
    {
        return m_Buffer != nullptr && m_Size > 0;
    }

    operator bool() const
    {
        return valid();
    }

    friend bool
    operator==(KAlloc const& lhs, KAlloc const& rhs)
    {
        if ( lhs.m_Size != rhs.m_Size || lhs.m_PoolTag != rhs.m_PoolTag )
        {
            return false;
        }

        return ::RtlCompareMemory((PVOID)lhs.m_Buffer, (PVOID)rhs.m_Buffer, lhs.m_Size);
    }

protected:
    virtual void
    allocate(const usize sz)
    {
        if ( sz > 0 )
        {
            auto p = ::ExAllocatePoolWithTag(m_PoolType, sz, m_PoolTag);
            if ( !p )
            {
                ::ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

            m_Buffer = reinterpret_cast<T>(p);
            ::RtlSecureZeroMemory((PVOID)m_Buffer, sz);
        }
        dbg("KAlloc::allocate(%d) = %p", sz, m_Buffer);
    }

    virtual void
    free()
    {
        dbg("KAlloc::free(%p)", m_Buffer);
        if ( valid() )
        {
            ::RtlSecureZeroMemory((PUCHAR)m_Buffer, m_Size);
            ::ExFreePoolWithTag(m_Buffer, m_PoolTag);
            m_Buffer  = nullptr;
            m_PoolTag = 0;
            m_Size    = 0;
        }
    }

    T m_Buffer;
    usize m_Size;
    u32 m_PoolTag;
    POOL_TYPE m_PoolType;
};


class KUnicodeString
{
public:
    ///
    /// @brief Construct a new KUnicodeString object for a widechar buffer and length
    ///
    /// @param src pointer to the beginning of the string. The buffer **MUST** include a null terminator
    /// @param srcsz the number of bytes to use to store the unicode string, including the null terminator
    /// @param type the pool type to store the buffer in
    ///
    KUnicodeString(const wchar_t* src, const u16 srcsz, const POOL_TYPE type = NonPagedPoolNx) :
        m_UnicodeString {},
        m_StringBuffer {KAlloc<wchar_t*>(srcsz, CFB_DEVICE_TAG, type)}
    {
        ::memcpy(m_StringBuffer.get(), src, srcsz);
        ::RtlInitUnicodeString(&m_UnicodeString, m_StringBuffer.get());

        dbg("KUnicodeString::KUnicodeString('%wZ', length=%lluB, capacity=%lluB)",
            &m_UnicodeString,
            length(),
            capacity());
    }

    ///
    ///@brief Construct a new KUnicodeString object from a pointer to a UNICODE_STRING
    ///
    ///@param src
    ///@param type
    ///
    KUnicodeString(const PUNICODE_STRING src, const POOL_TYPE type = NonPagedPoolNx) :
        m_UnicodeString {},
        m_StringBuffer {KAlloc<wchar_t*>(src->MaximumLength, CFB_DEVICE_TAG, type)}
    {
        ::memcpy(m_StringBuffer.get(), src->Buffer, src->Length);
        ::RtlInitUnicodeString(&m_UnicodeString, m_StringBuffer.get());

        dbg("KUnicodeString::KUnicodeString('%wZ', length=%lluB, capacity=%lluB)",
            &m_UnicodeString,
            length(),
            capacity());
    }

    ///
    ///@brief Default constructor
    ///
    ///
    KUnicodeString() : m_UnicodeString {}
    {
    }

    ///
    ///@brief Destroy the KUnicodeString object
    ///
    ~KUnicodeString()
    {
        dbg("KUnicodeString::~KUnicodeString(%p)", m_UnicodeString);
        // deallocation of `m_StringBuffer` is handled by KAlloc
    }

    KUnicodeString(const KUnicodeString&) = delete;
    KUnicodeString(KUnicodeString&&)      = delete;
    KUnicodeString&
    operator=(const KUnicodeString& other) noexcept = delete;

    KUnicodeString&
    operator=(KUnicodeString&& other) noexcept
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
    operator==(KUnicodeString const& other)
    {
        return length() == other.length() && ::RtlCompareUnicodeString(get(), other.get(), true) == 0;
    }

    bool
    operator==(PUNICODE_STRING const& other)
    {
        return length() == other->Length && ::RtlCompareUnicodeString(get(), other, true) == 0;
    }

    const wchar_t*
    data() const
    {
        return m_UnicodeString.Buffer;
    }

    ///
    ///@brief Get a pointer to the PUNICODE_STRING
    ///
    ///@return const PUNICODE_STRING
    ///
    const PUNICODE_STRING
    get() const
    {
        return const_cast<PUNICODE_STRING>(&m_UnicodeString);
    }

    ///
    /// @brief Get the length of the string (i.e. number of characters)
    ///
    /// @return const usize
    ///
    const usize
    length() const
    {
        return m_UnicodeString.Length;
    }

    const usize
    capacity() const
    {
        return m_UnicodeString.MaximumLength;
    }

protected:
    UNICODE_STRING m_UnicodeString;
    KAlloc<wchar_t*> m_StringBuffer;
};


///
/// @brief Wrapper for KMUTEX
///
class KMutex
{
public:
    ///
    /// @brief
    ///
    KMutex();

    ///
    /// @brief
    ///
    ~KMutex();

    ///
    /// @brief
    ///
    void
    Lock();

    ///
    /// @brief
    ///
    void
    Unlock();

private:
    KMUTEX _mutex = {0};
};


///
/// @brief Wrapper for FAST_MUTEXes
///
///
class KFastMutex
{

public:
    ///
    /// @brief
    ///
    KFastMutex();

    ///
    /// @brief
    ///
    ~KFastMutex();

    ///
    /// @brief
    ///
    void
    Lock();

    ///
    /// @brief
    ///
    void
    Unlock();

private:
    FAST_MUTEX _mutex = {0};
};


///
/// @brief Wrapper for kernel critical region
///
///
class KCriticalRegion
{

public:
    ///
    /// @brief Construct a new KCriticalRegion object
    ///
    KCriticalRegion();

    ///
    /// @brief Destroy the KCriticalRegion object
    ///
    ~KCriticalRegion();

    ///
    /// @brief
    ///
    void
    Lock();

    ///
    /// @brief
    ///
    void
    Unlock();

private:
    ERESOURCE _mutex = {0};
};


///
/// @brief Wrapper for KSPIN_LOCK
///
class KSpinLock
{
public:
    ///
    /// @brief
    ///
    KSpinLock();

    ///
    /// @brief
    ///
    ~KSpinLock();

    ///
    /// @brief
    ///
    void
    Lock();

    ///
    /// @brief
    ///
    void
    Unlock();

private:
    KSPIN_LOCK _SpinLock = 0;
    KIRQL _OldIrql       = 0;
};


///
/// @brief Wrapper for queued KSPIN_LOCK
///
class KQueuedSpinLock
{
public:
    KQueuedSpinLock();

    ~KQueuedSpinLock();

    void
    Lock();

    void
    Unlock();

private:
    KSPIN_LOCK _SpinLock                = 0;
    KLOCK_QUEUE_HANDLE _LockQueueHandle = {0};
};


///
/// @brief Linked list class: to be able to be linked, `T` class must have a LIST_ENTRY member
/// named `Next`
///
/// @tparam T
///
template<typename T>
class LinkedList
{
public:
    LinkedList() : m_TotalEntry(0), m_Mutex()
    {
        ::InitializeListHead(&m_ListHead);
    };

    usize
    Size() const
    {
        return m_TotalEntry;
    }

    void
    PushBack(T* NewEntry)
    {
        ScopedLock lock(m_Mutex);
        ::InsertTailList(&m_ListHead, &NewEntry->Next);
        m_TotalEntry++;
    }

    void
    PushFront(T* NewEntry)
    {
        ScopedLock lock(m_Mutex);
        ::InsertHeadList(&m_ListHead, &NewEntry->Next);
        m_TotalEntry++;
    }

    void
    operator+=(T* NewEntry)
    {
        PushBack(NewEntry);
    }

    bool
    Remove(T* Entry)
    {
        ScopedLock lock(m_Mutex);
        bool bSuccess = ::RemoveEntryList(&Entry->Next);
        if ( bSuccess )
        {
            m_TotalEntry--;
        }
        return bSuccess;
    }

    void
    operator-=(T* Entry)
    {
        Remove(Entry);
    }

    T*
    PopFront()
    {
        ScopedLock lock(m_Mutex);
        if ( m_TotalEntry == 0 )
        {
            return nullptr;
        }
        auto LastEntry = ::RemoveHeadList(&m_ListHead);
        if ( LastEntry == &m_ListHead )
        {
            return nullptr;
        }
        auto LastItem = CONTAINING_RECORD(LastEntry, T, Next);
        m_TotalEntry--;
        return LastItem;
    }

    T*
    PopBack()
    {
        ScopedLock lock(m_Mutex);
        if ( Size() == 0 )
        {
            return nullptr;
        }
        auto LastEntry = ::RemoveTailList(&m_ListHead);
        if ( LastEntry == &m_ListHead )
        {
            return nullptr;
        }
        auto LastItem = CONTAINING_RECORD(LastEntry, T, Next);
        m_TotalEntry--;
        return LastItem;
    }

    template<typename N>
    T*
    Find(N condition)
    {
        ScopedLock lock(m_Mutex);
        if ( !::IsListEmpty(&m_ListHead) )
        {
            for ( PLIST_ENTRY Entry = m_ListHead.Flink; Entry != &m_ListHead; Entry = Entry->Flink )
            {
                auto CurrentItem = CONTAINING_RECORD(Entry, T, Next);
                if ( condition(CurrentItem) == true )
                {
                    return CurrentItem;
                }
            }
        }
        return nullptr;
    }

    template<typename L>
    bool
    ForEach(L lambda)
    {
        ScopedLock lock(m_Mutex);
        bool bSuccess = true;
        if ( !::IsListEmpty(&m_ListHead) )
        {
            for ( PLIST_ENTRY Entry = m_ListHead.Flink; Entry != &m_ListHead; Entry = Entry->Flink )
            {
                auto CurrentItem = CONTAINING_RECORD(Entry, T, Next);
                bSuccess &= lambda(CurrentItem);
            }
        }
        return bSuccess;
    }


private:
    KFastMutex m_Mutex;
    LIST_ENTRY m_ListHead;
    usize m_TotalEntry;
};

///
/// @brief Basic implementation of unique pointer for the kernel
///
/// @tparam T
///
template<typename T>
class UniquePointer
{
public:
    UniquePointer() : m_data(nullptr)
    {
    }

    explicit UniquePointer(T* data) : m_data(data)
    {
    }

    ~UniquePointer()
    {
        delete m_data;
    }

    UniquePointer(std::nullptr_t) : m_data(nullptr)
    {
    }

    UniquePointer&
    operator=(std::nullptr_t)
    {
        reset();
        return *this;
    }

    UniquePointer(UniquePointer&& moving) noexcept : m_data(nullptr)
    {
        moving.swap(*this);
    }

    UniquePointer&
    operator=(UniquePointer&& moving) noexcept
    {
        moving.swap(*this);
        return *this;
    }

    template<typename U>
    UniquePointer(UniquePointer<U>&& moving)
    {
        UniquePointer<T> tmp(moving.release());
        tmp.swap(*this);
    }

    template<typename U>
    UniquePointer&
    operator=(UniquePointer<U>&& moving)
    {
        UniquePointer<T> tmp(moving.release());
        tmp.swap(*this);
        return *this;
    }

    UniquePointer(UniquePointer const&) = delete;

    UniquePointer&
    operator=(UniquePointer const&) = delete;

    T*
    operator->() const
    {
        return m_data;
    }

    T&
    operator*() const
    {
        return *m_data;
    }

    T*
    get() const
    {
        return m_data;
    }

    explicit operator bool() const
    {
        return m_data;
    }

    T*
    release() noexcept
    {
        T* result = nullptr;
        swap(result, m_data);
        return result;
    }

    void
    swap(UniquePointer& src) noexcept
    {
        InterlockedExchangePointer((PVOID*)m_data, (PVOID)src.m_data);
    }

    void
    reset()
    {
        T* tmp = release();
        delete tmp;
    }

private:
    T* m_data;
};


template<typename T>
void
swap(UniquePointer<T>& lhs, UniquePointer<T>& rhs)
{
    lhs.swap(rhs);
}

///
/// @brief Basic implementation of smart pointer for the kernel
///
/// @tparam T
///
template<typename T>
class SharedPointer
{

public:
    SharedPointer(T* _ptr) : m_Count(nullptr), m_Pointer(_ptr)
    {
        m_Count = new int(1);
    }

    ~SharedPointer()
    {
        ScopedLock lock(m_Mutex);

        DecrementCounter();

        if ( count() == 0 )
        {
            delete m_Pointer;
            delete m_Count;
        }
    }

    SharedPointer(const SharedPointer<T>& other)
    {
        ScopedLock lock(m_Mutex);
        m_Pointer = other.m_Pointer;
        m_Count   = other.count();
        IncrementCounter();
    }

    SharedPointer<T>&
    operator=(const SharedPointer<T>& other)
    {
        ScopedLock lock(m_Mutex);
        m_Pointer = other.m_Pointer;
        m_Count   = other.m_Count;
        IncrementCounter();
        return *this;
    }

    usize
    count() const
    {
        ScopedLock lock(m_Mutex);
        return (m_Count != nullptr) ? *m_Count : 0;
    }

    T*
    get()
    {
        ScopedLock lock(m_Mutex);
        return m_Pointer;
    }

    T*
    operator*()
    {
        return get();
    }

    T*
    operator->()
    {
        return get();
    }

protected:
    void
    IncrementCounter()
    {
        InterlockedIncrement(&m_Count);
    }

    void
    DecrementCounter()
    {
        InterlockedDecrement(&m_Count);
    }

private:
    T* m_Pointer;
    usize* m_Count;
    KFastMutex m_Mutex;
};


} // namespace CFB::Driver::Utils
