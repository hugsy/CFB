#include "CapturedIrp.hpp"

#include "Native.hpp"


namespace CFB::Driver
{

NTSTATUS
CapturedIrp::CapturePreCallData(_In_ PIRP Irp)
{
    NTSTATUS Status          = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    if ( Method != -1 )
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    do
    {
        if ( (Stack->MajorFunction == IRP_MJ_DEVICE_CONTROL ||
              Stack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
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
CapturedIrp::GetDeviceName()
{
    NTSTATUS Status    = STATUS_SUCCESS;
    ULONG ReturnLength = -1;
    Utils::KAlloc<POBJECT_NAME_INFORMATION> DeviceNameInfo;

    //
    // Query the exact size
    //
    Status = ::ObQueryNameString(DeviceObject, nullptr, 0, &ReturnLength);
    if ( Status != STATUS_INFO_LENGTH_MISMATCH )
    {
        return Status;
    }

    //
    // Get the value, store it in the member
    //
    DeviceNameInfo = Utils::KAlloc<POBJECT_NAME_INFORMATION>(sizeof(OBJECT_NAME_INFORMATION) + ReturnLength);
    Status         = ::ObQueryNameString(DeviceObject, DeviceNameInfo.get(), DeviceNameInfo.size(), &ReturnLength);
    if ( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    DeviceName = Utils::KUnicodeString(&DeviceNameInfo.get()->Name);
    return STATUS_SUCCESS;
}

NTSTATUS
CapturedIrp::GetProcessName()
{
    PEPROCESS Process = nullptr;
    NTSTATUS Status   = STATUS_UNSUCCESSFUL;

    Status = ::PsLookupProcessByProcessId(UlongToHandle(Pid), &Process);
    if ( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    PSTR lpProcessName = ::PsGetProcessImageFileName(Process);
    if ( !lpProcessName )
    {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    CANSI_STRING aStr;
    Status = ::RtlInitAnsiStringEx(&aStr, lpProcessName);
    if ( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    UNICODE_STRING uStr;
    Status = ::RtlAnsiStringToUnicodeString(&uStr, &aStr, true);

    ProcessName = Utils::KUnicodeString(&uStr);
    return Status;
}

NTSTATUS
CapturedIrp::GetDriverName(HookedDriver* const Driver)
{
    DriverName = Utils::KUnicodeString(Driver->Path.get());
    return STATUS_UNSUCCESSFUL;
}

} // namespace CFB::Driver
