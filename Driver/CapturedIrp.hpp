#pragma once

#include "Common.hpp"
#include "DriverUtils.hpp"
#include "HookedDriver.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{

class CapturedIrp
{
public:
    enum class IrpType : u8
    {
        Irp          = 0x00,
        FastIo_Ioctl = 0x80,
        FastIo_Read  = 0x81,
        FastIo_Write = 0x82,
    };

    enum class Flags : u32
    {
        UseOutputBuffer  = (1 << 0),
        UseInputBuffer   = (1 << 1),
        InitQueueMessage = (1 << 2),
    };


    CapturedIrp(const IrpType Type, const PDEVICE_OBJECT DeviceObject);

    ~CapturedIrp();

    static void*
    operator new(usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(NonPagedPoolNx, sz, CFB_DEVICE_TAG);
        if ( Memory )
        {
            dbg("Allocating CapturedIrp at %p", Memory);
            ::RtlSecureZeroMemory(Memory, sz);
        }
        else
        {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }
        return Memory;
    }

    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating CapturedIrp at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }

    NTSTATUS
    CapturePreCallData(_In_ PIRP Irp);

    NTSTATUS
    CapturePostCallData(_In_ PIRP Irp, _In_ NTSTATUS ReturnedIoctlStatus);

    NTSTATUS
    CapturePreCallFastIoData(_In_opt_ PVOID InputBuffer, _In_ ULONG IoControlCode);

    NTSTATUS
    CapturePostCallFastIoData(_Out_opt_ PVOID OutputBuffer);

    usize const
    DataSize();

    usize const
    InputDataSize() const;

    usize const
    OutputDataSize() const;

    HookedDriver* const
    AssociatedDriver() const;

    LIST_ENTRY Next;

private:
    LARGE_INTEGER m_TimeStamp;
    u8 m_Irql;
    IrpType m_Type;
    u8 m_MajorFunction;
    u32 m_IoctlCode;
    u32 m_Pid;
    u32 m_Tid;
    NTSTATUS m_Status;
    Utils::KAlloc<u8*> m_InputBuffer;
    Utils::KAlloc<u8*> m_OutputBuffer;
    PDEVICE_OBJECT m_DeviceObject;
    Utils::KUnicodeString m_DriverName;
    Utils::KUnicodeString m_DeviceName;
    Utils::KUnicodeString m_ProcessName;
    HookedDriver* m_Driver;
};

} // namespace CFB::Driver
