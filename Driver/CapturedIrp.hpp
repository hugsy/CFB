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
        ProcessName(),
        DeviceName(),
        DriverName(),
        MajorFunction(0)
    {
        ULONG Method             = -1;
        PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

        //
        // Basic info
        //
        KeQuerySystemTime(&TimeStamp);

        MajorFunction = Stack->MajorFunction;
        if ( !NT_SUCCESS(GetDeviceName()) )
        {
            RtlInitUnicodeString(&DeviceName, L"Unknown device");
        }
        if ( !NT_SUCCESS(GetDriverName(HookedDriver)) )
        {
            ::RtlInitUnicodeString(&DriverName, L"Unknown driver");
        }
        if ( !NT_SUCCESS(GetProcessName()) )
        {
            ::RtlInitUnicodeString(&ProcessName, L"Unknown process");
        }

        switch ( MajorFunction )
        {
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            InputBufferLength  = Stack->Parameters.DeviceIoControl.InputBufferLength;
            OutputBufferLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;
            IoctlCode          = Stack->Parameters.DeviceIoControl.IoControlCode;
            Data               = Utils::KAlloc<u8*>(InputBufferLength);
            Method             = METHOD_FROM_CTL_CODE(IoctlCode);
            break;

        case IRP_MJ_WRITE:
            InputBufferLength  = Stack->Parameters.Write.Length;
            OutputBufferLength = 0;
            Data               = Utils::KAlloc<u8*>(InputBufferLength);
            break;

        case IRP_MJ_READ:
            InputBufferLength  = 0;
            OutputBufferLength = Stack->Parameters.Read.Length;
            Data               = Utils::KAlloc<u8*>(OutputBufferLength);
            Method             = (DeviceObject->Flags & DO_BUFFERED_IO) ? METHOD_BUFFERED :
                                 (DeviceObject->Flags & DO_DIRECT_IO)   ? METHOD_IN_DIRECT :
                                                                          -1;
            break;

        default:
            break;
        }

        if ( Method != -1 )
        {
            GetIrpData(Irp, Method);
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


private:
    NTSTATUS
    GetIrpData(_In_ PIRP Irp, _In_ ULONG Method);

    NTSTATUS
    GetDeviceName();

    NTSTATUS
    GetProcessName();

    NTSTATUS
    GetDriverName(const HookedDriver* Driver);

    LIST_ENTRY Next;
    LARGE_INTEGER TimeStamp;
    u8 Irql;
    Type Type;
    u8 MajorFunction;
    u32 IoctlCode;
    u32 Pid;
    u32 Tid;
    u32 InputBufferLength;
    u32 OutputBufferLength;
    wchar_t wsDriverName[CFB_DRIVER_MAX_PATH];
    wchar_t wsDeviceName[CFB_DRIVER_MAX_PATH];
    wchar_t wsProcessName[CFB_DRIVER_MAX_PATH];
    NTSTATUS Status;
    Utils::KAlloc<u8*> Data;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DriverName;
    UNICODE_STRING DeviceName;
    UNICODE_STRING ProcessName;
};
} // namespace CFB::Driver
