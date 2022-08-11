#pragma once

#include "Common.hpp"
#include "Log.hpp"


void* __cdecl
operator new(size_t Size, POOL_TYPE PoolType = NonPagedPoolNx, ULONG PoolTag = CFB_DEVICE_TAG);

void __cdecl
operator delete(void* Memory, usize Size);

void* __cdecl
operator new[](size_t Size, POOL_TYPE PoolType);

void __cdecl
operator delete[](void* Memory, usize Size);

namespace CFB::Driver::Utils
{

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
        dbg("Scope lock: %p", &_lock);
    }

    ~ScopedLock()
    {
        _lock.Unlock();
        dbg("Scope unlocking: %p", &_lock);
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
    ScopedIrql(KIRQL level) : _NewIrql(level)
    {
        KeRaiseIrql(_NewIrql, &_OldIrql);
        dbg("IRQL %s -> %s", str(_OldIrql), str(_NewIrql));
    }

    ~ScopedIrql()
    {
        ::KeLowerIrql(_OldIrql);
        dbg("IRQL %d <- %d", str(_OldIrql), str(_NewIrql));
    }

    const char*
    str(KIRQL Level) const
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

private:
    KIRQL _OldIrql;
    KIRQL _NewIrql;
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
        _type(type),
        _tag(tag),
        _sz(sz),
        _mem(nullptr)
    {
        if ( sz )
        {
            allocate(sz);
        }
    }

    ~KAlloc()
    {
        if ( _sz )
        {
            free();
        }
    }

    KAlloc(const KAlloc&) = delete;

    KAlloc&
    operator=(const KAlloc& other) noexcept
    {
        dbg("In KAlloc::operator=(const KAlloc&)");
        if ( this != &other )
        {
            _mem = other._mem;
            _tag = other._tag;
            _sz  = other._sz;
        }
        return *this;
    }

    KAlloc&
    operator=(KAlloc&& other) noexcept
    {
        dbg("In KAlloc::operator=(const KAlloc&&)");
        if ( this != &other )
        {
            free();
            _mem       = other._mem;
            _tag       = other._tag;
            _sz        = other._sz;
            other._mem = nullptr;
            other._tag = 0;
            other._sz  = 0;
        }
        return *this;
    }

    const T
    get() const
    {
        return _mem;
    }

    const usize
    size() const
    {
        return _sz;
    }

    operator bool() const
    {
        return _mem != nullptr && _sz > 0;
    }

    friend bool
    operator==(KAlloc const& lhs, KAlloc const& rhs)
    {
        if ( lhs._sz != rhs._sz || lhs._tag != rhs._tag )
            return false;

        return ::RtlCompareMemory((PVOID)lhs._mem, (PVOID)rhs._mem, lhs._sz);
    }

protected:
    virtual void
    allocate(const usize sz)
    {
        if ( sz >= MAXUSHORT )
        {
            ::ExRaiseStatus(STATUS_INVALID_PARAMETER_1);
            return;
        }

        if ( _sz > 0 )
        {
            auto p = ::ExAllocatePoolWithTag(_type, _sz, _tag);
            if ( !p )
            {
                ::ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

            _mem = reinterpret_cast<T>(p);
            ::RtlSecureZeroMemory((PVOID)_mem, _sz);

            dbg("KAlloc::allocate(%d) = %p", sz, _mem);
        }
        else
        {
            dbg("KAlloc::allocate(%d)", sz);
        }
    }

    virtual void
    free()
    {
        dbg("KAlloc::free(%p)", _mem);
        if ( _mem != nullptr && _sz > 0 )
        {
            ::RtlSecureZeroMemory((PUCHAR)_mem, _sz);
            ::ExFreePoolWithTag(_mem, _tag);
            _mem = nullptr;
            _tag = 0;
            _sz  = 0;
        }
    }

    T _mem;
    usize _sz;
    u32 _tag;
    POOL_TYPE _type;
};


class KUnicodeString
{
public:
    KUnicodeString(const PUNICODE_STRING src, const POOL_TYPE type = NonPagedPoolNx);

    KUnicodeString(const wchar_t* src, const POOL_TYPE type = NonPagedPoolNx);

    ~KUnicodeString();

    KUnicodeString&
    operator=(KUnicodeString&& other) noexcept
    {
        dbg("In KUnicodeString::operator=(const KUnicodeString&&)");
        if ( this != &other )
        {
            _len    = other._len;
            _buffer = other._buffer;
            ::RtlCopyMemory(&_str, other.get(), sizeof(UNICODE_STRING));
        }
        return *this;
    }

    friend bool
    operator==(KUnicodeString const& lhs, KUnicodeString const& rhs)
    {
        return lhs._len == rhs._len && lhs._buffer == rhs._buffer;
    }

    bool
    operator==(PUNICODE_STRING const& uString)
    {
        return *this == KUnicodeString(uString->Buffer);
    }

    const PUNICODE_STRING
    get();

    ///
    /// @brief Get the length of the string (i.e. number of characters)
    ///
    /// @return const usize
    ///
    const usize
    length() const;

    ///
    /// @brief Get the size of the buffer (i.e. number of bytes)
    ///
    /// @return const usize
    ///
    const usize
    size() const;

protected:
    usize _len;
    UNICODE_STRING _str;
    KAlloc<wchar_t*> _buffer;
};


///
/// @brief Wrapper for KMUTEX
///
class KMutex
{
public:
    KMutex();

    ~KMutex();

    void
    Lock();

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
    KFastMutex();

    ~KFastMutex();

    void
    Lock();

    void
    Unlock();

private:
    FAST_MUTEX _mutex = {0};
};


///
/// @brief Wrapper for KSPIN_LOCK
///
class KSpinLock
{
public:
    KSpinLock();

    ~KSpinLock();

    void
    Lock();

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
            m_TotalEntry--;
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
            return nullptr;
        auto LastEntry = ::RemoveHeadList(&m_ListHead);
        if ( LastEntry == &m_ListHead )
            return nullptr;
        auto LastItem = CONTAINING_RECORD(LastEntry, T, Next);
        m_TotalEntry--;
        return LastItem;
    }

    T*
    PopBack()
    {
        ScopedLock lock(m_Mutex);
        if ( Size() == 0 )
            return nullptr;
        auto LastEntry = ::RemoveTailList(&m_ListHead);
        if ( LastEntry == &m_ListHead )
            return nullptr;
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
        ::RtlSecureZeroMemory(m_data, sizeof(T));
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
        m_Count   = other.count;
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
