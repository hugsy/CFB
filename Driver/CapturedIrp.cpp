#include "CapturedIrp.hpp"

#include "Context.hpp"
#include "Native.hpp"

namespace CFB::Driver
{

CapturedIrp::CapturedIrp(const CapturedIrp::IrpType _Type, PDEVICE_OBJECT DeviceObject) :
    Type(_Type),
    Pid(::HandleToULong(::PsGetCurrentProcessId())),
    Tid(::HandleToULong(::PsGetCurrentThreadId())),
    Driver(nullptr),
    DriverName(L""),
    DeviceName(L""),
    ProcessName(L""),
    MajorFunction(0),
    InputBufferLength(0),
    OutputBufferLength(0),
    IoctlCode(0),
    InputBuffer(0),
    OutputBuffer(0)
{
    auto FilterByDeviceAddress = [&DeviceObject](const HookedDriver* h)
    {
        for ( PDEVICE_OBJECT CurrentDevice = h->DriverObject->DeviceObject; CurrentDevice;
              CurrentDevice                = CurrentDevice->NextDevice )
        {
            if ( CurrentDevice == DeviceObject )
                return true;
        }
        return false;
    };

    Driver = Globals->DriverManager.Entries.Find(FilterByDeviceAddress);
    if ( Driver == nullptr )
    {
        //
        // This is really bad: if here the interception routine got called by a non-hooked driver
        // Could be a bad pointer restoration. Anyway, we log and fail for now.
        //
        err("Failed to find a HookedDriver object associated to the received IRP.\n"
            "This could indicates a corruption of the hooked driver list, you should probably reboot...\n");
        ::ExRaiseStatus(STATUS_NOT_FOUND);
    }

    KeQuerySystemTime(&TimeStamp);

    //
    // Set the Device name
    //
    {
        ULONG ReturnLength;
        Utils::KAlloc<POBJECT_NAME_INFORMATION> DeviceNameInfo;

        //
        // Query the exact size
        //
        NTSTATUS Status = ::ObQueryNameString(DeviceObject, nullptr, 0, &ReturnLength);
        if ( Status != STATUS_INFO_LENGTH_MISMATCH )
        {
            ::ExRaiseStatus(Status);
        }

        //
        // Get the value, store it in the member
        //
        DeviceNameInfo = Utils::KAlloc<POBJECT_NAME_INFORMATION>(sizeof(OBJECT_NAME_INFORMATION) + ReturnLength);
        Status         = ::ObQueryNameString(DeviceObject, DeviceNameInfo.get(), DeviceNameInfo.size(), &ReturnLength);
        if ( !NT_SUCCESS(Status) )
        {
            ::ExRaiseStatus(Status);
        }

        DeviceName = Utils::KUnicodeString(&DeviceNameInfo.get()->Name);
    }

    //
    // Set the driver name
    //
    {
        DriverName = Utils::KUnicodeString(Driver->Path.get());
    }

    //
    // Set the process name
    //
    {
        PEPROCESS Process = nullptr;
        NTSTATUS Status   = ::PsLookupProcessByProcessId(UlongToHandle(Pid), &Process);
        if ( !NT_SUCCESS(Status) )
        {
            ::ExRaiseStatus(Status);
        }

        PSTR lpProcessName = ::PsGetProcessImageFileName(Process);
        if ( !lpProcessName )
        {
            ::ExRaiseStatus(STATUS_OBJECT_NAME_NOT_FOUND);
        }

        CANSI_STRING aStr;
        Status = ::RtlInitAnsiStringEx(&aStr, lpProcessName);
        if ( !NT_SUCCESS(Status) )
        {
            ::ExRaiseStatus(Status);
        }

        UNICODE_STRING uStr;
        Status = ::RtlAnsiStringToUnicodeString(&uStr, &aStr, true);
        if ( !NT_SUCCESS(Status) )
        {
            ::ExRaiseStatus(Status);
        }

        ProcessName = Utils::KUnicodeString(&uStr);
    }
}

CapturedIrp::~CapturedIrp()
{
    dbg("CapturedIrp::~CapturedIrp ");
}


NTSTATUS
CapturedIrp::CapturePreCallData(_In_ PIRP Irp)
{
    NTSTATUS Status          = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);
    ULONG Method             = 0;
    MajorFunction            = Stack->MajorFunction;

    //
    // Determine & allocate the input/output buffer sizes from the IRP for "normal" IOCTLs
    //
    switch ( MajorFunction )
    {
    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        InputBufferLength  = Stack->Parameters.DeviceIoControl.InputBufferLength;
        OutputBufferLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;
        IoctlCode          = Stack->Parameters.DeviceIoControl.IoControlCode;
        InputBuffer        = Utils::KAlloc<u8*>(InputBufferLength);
        OutputBuffer       = Utils::KAlloc<u8*>(OutputBufferLength);
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
        return STATUS_INVALID_PARAMETER_1;
    }

    //
    // Now, copy the input/output buffer content to the CapturedIrp object
    //
    do
    {
        if ( (MajorFunction == IRP_MJ_DEVICE_CONTROL || MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
             Method == METHOD_NEITHER )
        {
            if ( Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16) )
                RtlCopyMemory(
                    InputBuffer.get(),
                    Stack->Parameters.DeviceIoControl.Type3InputBuffer,
                    InputBuffer.size());
            else
                Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if ( Method == METHOD_BUFFERED )
        {
            if ( Irp->AssociatedIrp.SystemBuffer )
                RtlCopyMemory(InputBuffer.get(), Irp->AssociatedIrp.SystemBuffer, InputBuffer.size());
            else
                Status = STATUS_INVALID_PARAMETER_1;
            break;
        }

        if ( Method == METHOD_IN_DIRECT || Method == METHOD_OUT_DIRECT )
        {
            if ( !Irp->MdlAddress )
            {
                Status = STATUS_INVALID_PARAMETER_2;
                break;
            }

            PVOID pDataAddr = ::MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
            if ( !pDataAddr )
            {
                Status = STATUS_INVALID_PARAMETER_3;
                break;
            }

            RtlCopyMemory(InputBuffer.get(), pDataAddr, InputBuffer.size());
        }

    } while ( false );

    return Status;
}


NTSTATUS
CapturedIrp::CapturePostCallData(_In_ PIRP Irp, _In_ NTSTATUS IoctlStatus)
{
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);
    PVOID UserBuffer         = nullptr;

    Status = IoctlStatus;

    //
    // Check if the operation supports having output buffer
    //
    switch ( Stack->MajorFunction )
    {
    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        UserBuffer = Irp->UserBuffer;
        break;

    case IRP_MJ_READ:
        if ( Irp->MdlAddress )
            UserBuffer = ::MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
        else
            UserBuffer = Irp->UserBuffer;
        break;

    default:
        return STATUS_SUCCESS;
    }

    //
    // Check if there's actual data to be copied
    //
    if ( OutputBufferLength == 0 || UserBuffer == nullptr )
    {
        return STATUS_SUCCESS;
    }

    //
    // If here, just copy the buffer
    //
    RtlCopyMemory(OutputBuffer.get(), UserBuffer, OutputBufferLength);

    return STATUS_SUCCESS;
}


NTSTATUS
CapturedIrp::CapturePreCallFastIoData(_In_ PVOID InputBuffer, _In_ ULONG InputBufferLength, _In_ ULONG IoControlCode)
{
    return STATUS_SUCCESS;
}


NTSTATUS
CapturedIrp::CapturePostCallFastIoData(_In_ PVOID OutputBuffer, _In_ ULONG OutputBufferLength)
{
    return STATUS_SUCCESS;
}


usize const
CapturedIrp::DataSize()
{
    return InputDataSize() + OutputDataSize();
}


usize const
CapturedIrp::InputDataSize() const
{
    return InputBufferLength;
}


usize const
CapturedIrp::OutputDataSize() const
{
    return OutputBufferLength;
}

HookedDriver* const
CapturedIrp::AssociatedDriver() const
{
    return Driver;
}
} // namespace CFB::Driver
