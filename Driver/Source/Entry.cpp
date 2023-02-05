#include "Common.hpp"
#include "Context.hpp"
#include "IoctlCodes.hpp"
#include "Utils.hpp"


struct GlobalContext* Globals = nullptr;


NTSTATUS
static inline CompleteRequest(_In_ PIRP Irp, _In_ NTSTATUS Status, _In_ ULONG_PTR Information)
{
    Irp->IoStatus.Status      = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


EXTERN_C
NTSTATUS
IrpNotImplementedHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    CompleteRequest(Irp, STATUS_NOT_IMPLEMENTED, 0);
    return STATUS_NOT_IMPLEMENTED;
}


EXTERN_C
void
DriverUnloadRoutine(_In_ PDRIVER_OBJECT DriverObject)
{
    dbg("Unloading '%S'...", CFB_DEVICE_NAME);
    UNICODE_STRING SymLink = RTL_CONSTANT_STRING(CFB_DOS_DEVICE_PATH);
    ::IoDeleteSymbolicLink(&SymLink);
    ::IoDeleteDevice(DriverObject->DeviceObject);
    ok("Device '%S' deleted", CFB_DEVICE_NAME);

    delete Globals;

    ok("Context cleaned up");
    return;
}

///
/// @brief Callback routine for obtaining a handle to the device. Getting a handle requires the SeDebug privilege
///
/// @param DeviceObject
/// @param Irp
///
/// @return NTSTATUS
///
NTSTATUS
_Function_class_(DRIVER_DISPATCH) DriverCreateRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(pObject);

    NTSTATUS Status                        = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION lpStack             = ::IoGetCurrentIrpStackLocation(Irp);
    PIO_SECURITY_CONTEXT lpSecurityContext = lpStack->Parameters.Create.SecurityContext;


    //
    // Ensure the calling process has SeDebugPrivilege
    //
    UCHAR ucPrivilegesBuffer[FIELD_OFFSET(PRIVILEGE_SET, Privilege) + 2 * sizeof(LUID_AND_ATTRIBUTES)] = {0};

    PPRIVILEGE_SET lpRequiredPrivileges              = reinterpret_cast<PPRIVILEGE_SET>(ucPrivilegesBuffer);
    lpRequiredPrivileges->PrivilegeCount             = 1;
    lpRequiredPrivileges->Control                    = PRIVILEGE_SET_ALL_NECESSARY;
    lpRequiredPrivileges->Privilege[0].Luid.LowPart  = SE_DEBUG_PRIVILEGE;
    lpRequiredPrivileges->Privilege[0].Luid.HighPart = 0;
    lpRequiredPrivileges->Privilege[0].Attributes    = 0;
    lpRequiredPrivileges->Privilege[1].Luid.LowPart  = SE_LOAD_DRIVER_PRIVILEGE;
    lpRequiredPrivileges->Privilege[1].Luid.HighPart = 0;
    lpRequiredPrivileges->Privilege[1].Attributes    = 0;

    if ( ::SePrivilegeCheck(
             lpRequiredPrivileges,
             &lpSecurityContext->AccessState->SubjectSecurityContext,
             Irp->RequestorMode) == false )
    {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }
    else
    {
        auto ScopedSpinLock       = CFB::Driver::Utils::ScopedLock(Globals->ContextLock);
        PEPROCESS pCallingProcess = ::PsGetCurrentProcess();

        if ( Globals->Owner == nullptr )
        {
            //
            // If there's no process owner, affect one and increment the handle counter
            //
            /// TODO: add some sort of authentication process
            Globals->Owner = pCallingProcess;
            Globals->SessionId++;
            ok("Locked device to EPROCESS=%p, starting session=%d...", Globals->Owner, Globals->SessionId);
            Status = STATUS_SUCCESS;
        }
        else if ( pCallingProcess == Globals->Owner )
        {
            //
            // If the CreateFile() originates from the owner process, increment the handle counter
            //
            Status = STATUS_SUCCESS;
        }
        else
        {
            //
            // In any other case, simply reject
            //
            Status = STATUS_DEVICE_ALREADY_ATTACHED;
        }
    }

    return CompleteRequest(Irp, Status, 0);
}


///
/// @brief Callback routine when closing a handle to the device
///
/// @param DeviceObject
/// @param Irp
///
/// @return NTSTATUS
///
NTSTATUS
_Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(_In_ PDEVICE_OBJECT Device, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(Device);

    auto lock      = CFB::Driver::Utils::ScopedLock(Globals->ContextLock);
    Globals->Owner = nullptr;
    ok("Unlocked device...");
    return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


///
/// @brief Handle a IOCTL dipatcher for DeviceIoControl() from the broker
///
/// @param DeviceObject
/// @param Irp
///
/// @return NTSTATUS
///
NTSTATUS
_Function_class_(DRIVER_DISPATCH) DriverDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // This should never happen as we checked the process when getting the handle, but still
    //
    if ( PsGetCurrentProcess() != Globals->Owner )
    {
        warn("Refusing access from EPROCESS %p (expected %p)", PsGetCurrentProcess(), Globals->Owner);
        return CompleteRequest(Irp, STATUS_ACCESS_DENIED, 0);
    }

    NTSTATUS Status                 = STATUS_SUCCESS;
    PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);
    NT_ASSERT(CurrentStack);

    const ULONG dwIoctlCode           = CurrentStack->Parameters.DeviceIoControl.IoControlCode;
    const CFB::Comms::Ioctl IoctlCode = CFB::Comms::Ioctl(dwIoctlCode);
    PVOID InputBuffer                 = Irp->AssociatedIrp.SystemBuffer;
    const ULONG InputBufferLen  = min(CurrentStack->Parameters.DeviceIoControl.InputBufferLength, CFB_DRIVER_MAX_PATH);
    PVOID OutputBuffer          = Irp->AssociatedIrp.SystemBuffer;
    const ULONG OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG dwDataWritten         = 0;

    dbg("Attempting to process IOCTL %#x (IRQL=%d)", IoctlCode, ::KeGetCurrentIrql());

    switch ( IoctlCode )
    {
    case CFB::Comms::Ioctl::HookDriver:
    {
        auto DriverName = Utils::KUnicodeString(reinterpret_cast<wchar_t*>(InputBuffer), InputBufferLen);
        Status          = Globals->DriverManager.InsertDriver(DriverName.get());
        break;
    }

    case CFB::Comms::Ioctl::UnhookDriver:
    {
        auto DriverName = Utils::KUnicodeString(reinterpret_cast<wchar_t*>(InputBuffer), InputBufferLen);
        Status          = Globals->DriverManager.RemoveDriver(DriverName.get());
        break;
    }

    case CFB::Comms::Ioctl::GetNumberOfDrivers:
    {
        dwDataWritten = Globals->DriverManager.Items().Size();
        Status        = STATUS_SUCCESS;
        break;
    }

    case CFB::Comms::Ioctl::EnableMonitoring:
    case CFB::Comms::Ioctl::DisableMonitoring:
    {
        auto DriverName = Utils::KUnicodeString(reinterpret_cast<wchar_t*>(InputBuffer), InputBufferLen);
        Status          = Globals->DriverManager.SetMonitoringState(
            DriverName.get(),
            (IoctlCode == CFB::Comms::Ioctl::EnableMonitoring));
        break;
    }

    case CFB::Comms::Ioctl::SetEventPointer:
    {
        const HANDLE hEvent = *((PHANDLE)InputBuffer);
        Status              = Globals->IrpManager.SetEvent(hEvent);
        break;
    }

    case CFB::Comms::Ioctl::GetDriverInfo:
        warn("TODO");
        Status = STATUS_NOT_IMPLEMENTED;
        break;

    default:
        err("Received invalid IOCTL code 0x%08x", IoctlCode);
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    dbg("Routine for IOCTL 0x%08x returned with Status=0x%x", IoctlCode, Status);
    if ( !NT_SUCCESS(Status) )
    {
        err("IOCTL %#x returned %#x", IoctlCode, Status);
        dwDataWritten = 0;
    }

    return CompleteRequest(Irp, Status, dwDataWritten);
}


///
/// @brief Handles ReadFile() request to the device: this will effectively read the hooked IRPs back to the broker.
/// If read buffer length is zero, it will return the expected size for the
/// buffer.
///
/// @param DeviceObject
/// @param Irp
///
/// @return NTSTATUS
///
NTSTATUS
_Function_class_(DRIVER_DISPATCH) DriverReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION pStack = ::IoGetCurrentIrpStackLocation(Irp);
    if ( pStack == nullptr )
    {
        err("IoGetCurrentIrpStackLocation() failed (IRP %p)", Irp);
        return CompleteRequest(Irp, STATUS_INVALID_PARAMETER_1, 0);
    }

    const u32 RequestedBufferSize = pStack->Parameters.Read.Length;
    usize ExpectedBufferSize      = 0;
    usize DumpableIrpNumber       = 0;

    Globals->IrpManager.Items().ForEach(
        [&ExpectedBufferSize, &DumpableIrpNumber](CFB::Driver::CapturedIrp* Irp)
        {
            ExpectedBufferSize += Irp->Size();
            DumpableIrpNumber++;
            return true;
        });

    dbg("[DriverReadRoutine] RequestedBufferSize = %uB , ExpectedBufferSize = %uB , CurrentIrpNumber = %u",
        RequestedBufferSize,
        ExpectedBufferSize,
        DumpableIrpNumber);

    //
    // Otherwise, check RequestedBufferSize vs ExpectedBufferSize
    //
    if ( RequestedBufferSize < ExpectedBufferSize )
    {
        return CompleteRequest(Irp, STATUS_BUFFER_TOO_SMALL, ExpectedBufferSize);
    }

    NT_ASSERT(Irp->MdlAddress);

    PVOID Buffer = ::MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
    if ( Buffer == nullptr )
    {
        return CompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }

    //
    // Critical region for the IRP copy
    //
    {
        Utils::ScopedLock<Utils::KCriticalRegion>(Globals->IrpManager.CriticalRegion());
        const uptr BufferPointerBase = reinterpret_cast<uptr>(Buffer);
        const uptr BufferPointerHigh = BufferPointerBase + ExpectedBufferSize;
        uptr BufferPointer           = BufferPointerBase;

        for ( usize i = 0; i < DumpableIrpNumber; i++ )
        {
            //
            // Pop front the captured IRP
            //
            auto CurrentIrp = Globals->IrpManager.Pop();
            if ( CurrentIrp == nullptr )
            {
                err("Expected to find CapturedIrp %d/%d, but none was popped.", i, DumpableIrpNumber);
                return CompleteRequest(Irp, STATUS_BAD_DATA, 0);
            }

            //
            // Copy the header (always)
            //
            {
                const CFB::Comms::CapturedIrpHeader Header = CurrentIrp->ExportHeader();
                usize const DataSize                       = sizeof(CFB::Comms::CapturedIrpHeader);
                if ( BufferPointer + DataSize > BufferPointerHigh )
                {
                    return CompleteRequest(Irp, STATUS_BUFFER_OVERFLOW, 0);
                }

                RtlCopyMemory((PVOID)BufferPointer, &Header, DataSize);
                BufferPointer += DataSize;
            }

            //
            // Copy the IRP input buffer (if any)
            //
            {
                usize const DataSize = CurrentIrp->InputDataSize();
                if ( DataSize )
                {
                    if ( BufferPointer + DataSize > BufferPointerHigh )
                    {
                        return CompleteRequest(Irp, STATUS_BUFFER_OVERFLOW, 0);
                    }
                    RtlCopyMemory((PVOID)BufferPointer, CurrentIrp->InputBuffer(), DataSize);
                    BufferPointer += DataSize;
                }
            }

            //
            // Copy the IRP output buffer (if any)
            //
            {
                usize const DataSize = CurrentIrp->OutputDataSize();
                if ( DataSize )
                {
                    if ( BufferPointer + DataSize > BufferPointerHigh )
                    {
                        return CompleteRequest(Irp, STATUS_BUFFER_OVERFLOW, 0);
                    }
                    RtlCopyMemory((PVOID)BufferPointer, CurrentIrp->OutputBuffer(), DataSize);
                    BufferPointer += DataSize;
                }
            }
        }
    }

    return CompleteRequest(Irp, STATUS_SUCCESS, ExpectedBufferSize);
}


///
/// @brief Handle cleanup
///
NTSTATUS
_Function_class_(DRIVER_DISPATCH) DriverCleanup(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


///
/// @brief Driver entry function
///
EXTERN_C
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS Status             = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT DeviceObject = nullptr;
    UNICODE_STRING Name         = RTL_CONSTANT_STRING(CFB_DEVICE_PATH);
    UNICODE_STRING SymLink      = RTL_CONSTANT_STRING(CFB_DOS_DEVICE_PATH);

    //
    // Make sure we clean everything correctly on function exit
    //
    auto CleanupOnFailure = CFB::Driver::Utils::ScopedWrapper(
        Status,
        [&Status]()
        {
            if ( !NT_SUCCESS(Status) )
            {
                warn("Failed to initialize driver, cleaning up...");
                if ( Globals )
                {
                    if ( Globals->DeviceObject )
                    {
                        ::IoDeleteDevice(Globals->DeviceObject);
                    }

                    delete Globals;
                    Globals = nullptr;
                }
            }
        });

    info("Initializing global context and device...");
    Globals = new GlobalContext();

    for ( auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++ )
    {
        DriverObject->MajorFunction[i] = IrpNotImplementedHandler;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = DriverCreateRoutine;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DriverCloseRoutine;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControlRoutine;
    DriverObject->MajorFunction[IRP_MJ_READ]           = DriverReadRoutine;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DriverCleanup;
    DriverObject->DriverUnload                         = DriverUnloadRoutine;

    Status = ::IoCreateDevice(
        DriverObject,
        0,
        &Name,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        false, // ACL is managed by DriverCreateRoutine
        &DeviceObject);
    if ( !NT_SUCCESS(Status) )
    {
        err("Error creating device object (0x%08X)", Status);
        return Status;
    }

    ok("Device '%S' successfully created", CFB_DEVICE_NAME);

    Status = ::IoCreateSymbolicLink(&SymLink, &Name);
    if ( !NT_SUCCESS(Status) )
    {
        err("IoCreateSymbolicLink() failed: 0x%08X", Status);
        return Status;
    }

    ok("Symlink for '%S' created", CFB_DEVICE_NAME);

    DeviceObject->Flags |= DO_DIRECT_IO;
    DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

    Globals->DeviceObject = DeviceObject;
    Globals->DriverObject = DriverObject;

    err("Device initialization for '%S' done, use `%s` for debug logs",
        CFB_DEVICE_NAME,
        DML("ed nt !Kd_IHVDRIVER_Mask f"));
    return Status;
}
