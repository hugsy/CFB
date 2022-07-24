#pragma once

#include "Common.hpp"
#include "HookedDriver.hpp"

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{

class CapturedIrp
{
public:
    enum class Type : u8
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

    CapturedIrp(_In_ HookedDriver* HookedDriver, _In_ const PDEVICE_OBJECT _DeviceObject, _In_ const PIRP Irp) :
        Type(Type::Irp),
        Pid(::HandleToULong(::PsGetCurrentProcessId())),
        Tid(::HandleToULong(::PsGetCurrentThreadId())),
        DeviceObject(_DeviceObject),
        MajorFunction(0),
        DriverName(L""),
        DeviceName(L""),
        ProcessName(L""),
        Method(-1)
    {
        PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

        //
        // Basic info
        //
        KeQuerySystemTime(&TimeStamp);

        MajorFunction = Stack->MajorFunction;
        GetDeviceName();
        GetDriverName(HookedDriver);
        GetProcessName();

        switch ( MajorFunction )
        {
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            InputBufferLength  = Stack->Parameters.DeviceIoControl.InputBufferLength;
            OutputBufferLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;
            IoctlCode          = Stack->Parameters.DeviceIoControl.IoControlCode;
            InputBuffer        = Utils::KAlloc<u8*>(InputBufferLength);
            Method             = METHOD_FROM_CTL_CODE(IoctlCode);
            break;

        case IRP_MJ_WRITE:
            InputBufferLength  = Stack->Parameters.Write.Length;
            OutputBufferLength = 0;
            InputBuffer        = Utils::KAlloc<u8*>(InputBufferLength);
            break;

        case IRP_MJ_READ:
            InputBufferLength  = 0;
            OutputBufferLength = Stack->Parameters.Read.Length;
            OutputBuffer       = Utils::KAlloc<u8*>(OutputBufferLength);
            Method             = (DeviceObject->Flags & DO_BUFFERED_IO) ? METHOD_BUFFERED :
                                 (DeviceObject->Flags & DO_DIRECT_IO)   ? METHOD_IN_DIRECT :
                                                                          -1;
            break;

        default:
            break;
        }
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

    LIST_ENTRY Next;

    usize const
    DataSize()
    {
        return InputDataSize() + OutputDataSize();
    }

    usize const
    InputDataSize() const
    {
        return InputBufferLength;
    }

    usize const
    OutputDataSize() const
    {
        return OutputBufferLength;
    }

private:
    NTSTATUS
    GetDeviceName();

    NTSTATUS
    GetProcessName();

    NTSTATUS
    GetDriverName(HookedDriver* const Driver);

    LARGE_INTEGER TimeStamp;
    u8 Irql;
    Type Type;
    u8 MajorFunction;
    u32 IoctlCode;
    u32 Pid;
    u32 Tid;
    u32 InputBufferLength;
    u32 OutputBufferLength;
    NTSTATUS Status;
    Utils::KAlloc<u8*> InputBuffer;
    Utils::KAlloc<u8*> OutputBuffer;
    PDEVICE_OBJECT DeviceObject;
    Utils::KUnicodeString DriverName;
    Utils::KUnicodeString DeviceName;
    Utils::KUnicodeString ProcessName;
    ULONG Method;
};
} // namespace CFB::Driver
