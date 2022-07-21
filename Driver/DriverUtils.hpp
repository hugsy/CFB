#pragma once

#include "Common.hpp"
#include "Log.hpp"

///
/// @brief Basic fake exception dispatcher
///
/// @param Status
///
void __cdecl throws(NTSTATUS Status);

void* __cdecl
operator new(size_t Size, POOL_TYPE PoolType = PagedPool, ULONG PoolTag = CFB_DEVICE_TAG);

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

private:
    T& _lock;
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
/// @brief
///
/// @tparam T
///
template<typename T>
class KAlloc
{
public:
    KAlloc(const usize sz = 0, const u32 tag = CFB_DEVICE_TAG, POOL_TYPE type = PagedPool) :
        _tag(tag),
        _sz(sz),
        _mem(nullptr),
        _valid(false)
    {
        dbg("In KAlloc()");
        if ( !sz || sz >= MAXUSHORT )
        {
            return;
        }

        auto p = ::ExAllocatePoolWithTag(type, _sz, _tag);
        if ( p )
        {
            _mem = reinterpret_cast<T>(p);
            ::RtlSecureZeroMemory((PUCHAR)_mem, _sz);
            _valid = true;
        }
    }

    ~KAlloc()
    {
        if ( valid() )
        {
            __free();
        }
    }

    virtual void
    __free()
    {
        if ( _mem != nullptr )
        {
            ::RtlSecureZeroMemory((PUCHAR)_mem, _sz);
            ::ExFreePoolWithTag(_mem, _tag);
            _mem   = nullptr;
            _tag   = 0;
            _sz    = 0;
            _valid = false;
        }
    }

    KAlloc(const KAlloc&) = delete;

    KAlloc&
    operator=(const KAlloc&) = delete;

    KAlloc&
    operator=(KAlloc&& other) noexcept
    {
        if ( this != &other )
        {
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

    bool
    valid() const
    {
        return _valid;
    }

    operator bool() const
    {
        return _valid && _mem != nullptr && _sz > 0;
    }

    friend bool
    operator==(KAlloc const& lhs, KAlloc const& rhs)
    {
        return lhs._mem == rhs._mem && lhs._tag == rhs._tag;
    }

protected:
    T _mem;
    usize _sz;
    u32 _tag;
    bool _valid;
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
/// @brief
///
/// @tparam T
///
template<typename T>
class LinkedList
{
public:
    LinkedList() : m_TotalEntry(0), m_SpinLock()
    {
        ::InitializeListHead(&m_ListHead);
    };

    usize
    Size() const
    {
        return m_TotalEntry;
    }

    void
    Insert(T* NewEntry)
    {
        ScopedLock lock(m_SpinLock);
        ::InsertTailList(&m_ListHead, &NewEntry->Next);
        m_TotalEntry++;
    }

    void
    operator+=(T* NewEntry)
    {
        Insert(NewEntry);
    }

    bool
    Remove(T* Entry)
    {
        ScopedLock lock(m_SpinLock);
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
    PopTail()
    {
        ScopedLock lock(m_SpinLock);
        if ( Size() == 0 )
            return nullptr;
        auto LastEntry = ::RemoveTailList(&m_ListHead);
        auto LastItem  = CONTAINING_RECORD(LastEntry, T, Next);
        m_TotalEntry--;
        return LastItem;
    }

    template<typename N>
    T*
    Find(N condition)
    {
        ScopedLock lock(m_SpinLock);
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
        ScopedLock lock(m_SpinLock);
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

    void
    Reset()
    {
        ScopedLock lock(m_SpinLock);
        while ( !::IsListEmpty(&m_ListHead) )
        {
            ::RemoveHeadList(&m_ListHead);
        }
    }

private:
    KQueuedSpinLock m_SpinLock;
    LIST_ENTRY m_ListHead;
    usize m_TotalEntry;
};


///
/// @brief
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
        if ( !m_Count )
        {
            return;
        }
        (*count)--;
        if ( *m_Count == 0 )
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
        (*m_Count)++;
    }

    SharedPointer<T>&
    operator=(const SharedPointer<T>& other)
    {
        ScopedLock lock(m_Mutex);
        m_Pointer = other.m_Pointer;
        m_Count   = other.m_Count;
        (*m_Count)++;
        return *this;
    }

    size_t
    count() const
    {
        return (m_Count != nullptr) ? *m_Count : 0;
    }

    T*
    get()
    {
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

private:
    T* m_Pointer;
    usize* m_Count;
    KFastMutex m_Mutex;
};

} // namespace CFB::Driver::Utils
