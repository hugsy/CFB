#include "Common.hpp"
#include "Log.hpp"

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
        dbg("Scope unocking: %p", &_lock);
    }

private:
    T& _lock;
};


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
        if(valid())
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
            _mem = nullptr;
            _tag = 0;
            _sz  = 0;
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

    void
    Lock();

    void
    Unlock();

private:
    KMUTEX _mutex;
};


///
/// @brief Wrapper for KSPIN_LOCK
///
class KSpinLock
{
public:
    KSpinLock();
    void
    Lock();
    void
    Unlock();

private:
    KSPIN_LOCK _SpinLock = 0;
    KIRQL _OldIrql = 0;
};


///
/// @brief Wrapper for queued KSPIN_LOCK
///
class KQueuedSpinLock
{
public:
    KQueuedSpinLock();
    void
    Lock();
    void
    Unlock();

private:
    KSPIN_LOCK _SpinLock = 0;
    KLOCK_QUEUE_HANDLE _LockQueueHandle = { 0 };
};

}
