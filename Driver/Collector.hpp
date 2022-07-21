#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{

struct CapturedIrp
{
    enum class Type : u8
    {
        Irp          = 0x00,
        FastIo_Ioctl = 0x10,
        Fastio_Read  = 0x11,
        Fastio_Write = 0x12,
    };

    enum class Flags : u8
    {
        UseOutputBuffer  = (1 << 0),
        UseInputBuffer   = (1 << 1),
        InitQueueMessage = (1 << 2),
    };

    LIST_ENTRY Next;
    LARGE_INTEGER TimeStamp;
    u8 Irql;
    Type Type;
    u32 IoctlCode;
    u32 Pid;
    u32 Tid;
    u32 InputBufferLength;
    u32 OutputBufferLength;
    wchar_t DriverName[CFB_DRIVER_MAX_PATH];
    wchar_t DeviceName[CFB_DRIVER_MAX_PATH];
    wchar_t ProcessName[CFB_DRIVER_MAX_PATH];
    NTSTATUS Status;

    CapturedIrp()
    {
    }

    ~CapturedIrp()
    {
    }

    static void*
    operator new(usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(PagedPool, sz, CFB_DEVICE_TAG);
        if ( Memory )
        {
            dbg("Allocating CapturedIrp at %p", Memory);
            ::RtlSecureZeroMemory(Memory, sz);
        }
        return Memory;
    }

    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating CapturedIrp at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }
};

template<typename T>
struct DataCollector
{
    ///
    /// @brief Set when new data is pushed
    ///
    PKEVENT Event;

    ///
    /// @brief Item count
    ///
    usize Count;

    ///
    /// @brief
    ///
    Utils::LinkedList<T> Data;

    ///
    /// @brief
    ///
    Utils::KFastMutex Mutex;

    ///
    /// @brief Construct a new Data Collector object
    ///
    DataCollector() : Event(nullptr), Count(0), Data(), Mutex()
    {
    }

    ///
    /// @brief Destroy the Data Collector object
    ///
    ~DataCollector()
    {
        if ( Event )
        {
            //
            // Free the Event object
            //
            ::KeResetEvent(Event);
            ObDereferenceObject(Event);
        }
    }

    ///
    /// @brief Set the Event object
    ///
    /// @param hEvent
    /// @return true
    /// @return false
    ///
    bool
    SetEvent(HANDLE hEvent);

    ///
    /// @brief Push data to the back of the queue
    ///
    /// @return true
    /// @return false
    ///
    bool
    Push(T*);

    ///
    /// @brief Pop the front of the queue
    ///
    /// @return true
    /// @return false
    ///
    T*
    Pop();

    ///
    /// @brief Flush the data in the container
    ///
    void
    Clear();
};

} // namespace CFB::Driver
