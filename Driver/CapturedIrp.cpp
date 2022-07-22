#include "CapturedIrp.hpp"

#include "Native.hpp"


namespace CFB::Driver
{

NTSTATUS
CapturedIrp::GetIrpData(_In_ PIRP Irp, _In_ ULONG Method)
{
    NTSTATUS Status          = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    do
    {
        if ( (Stack->MajorFunction == IRP_MJ_DEVICE_CONTROL ||
              Stack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
             Method == METHOD_NEITHER )
        {
            if ( Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16) )
                RtlCopyMemory(Data.get(), Stack->Parameters.DeviceIoControl.Type3InputBuffer, Data.size());
            else
                Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if ( Method == METHOD_BUFFERED )
        {
            if ( Irp->AssociatedIrp.SystemBuffer )
                RtlCopyMemory(Data.get(), Irp->AssociatedIrp.SystemBuffer, Data.size());
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

            RtlCopyMemory(Data.get(), pDataAddr, Data.size());
        }

    } while ( false );

    return Status;
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

    ::RtlCopyUnicodeString(&DeviceName, &DeviceNameInfo.get()->Name);
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

    Status = ::RtlAnsiStringToUnicodeString(&ProcessName, &aStr, true);
    return Status;
}

NTSTATUS
CapturedIrp::GetDriverName(const HookedDriver* Driver)
{
    ::RtlCopyUnicodeString(&DriverName, &Driver->Path);
    return STATUS_UNSUCCESSFUL;
}

} // namespace CFB::Driver
