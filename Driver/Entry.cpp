#include "Common.hpp"
#include "Context.hpp"
#include "IoctlCodes.hpp"
#include "Utils.hpp"

#define DEVICE_NAME CFB_DEVICE_NAME
#define DEVICE_PATH CFB_DEVICE_PATH
#define DOS_DEVICE_PATH CFB_DOS_DEVICE_PATH

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
    dbg("Unloading '%S'...", DEVICE_NAME);
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(DOS_DEVICE_PATH);
    ::IoDeleteSymbolicLink(&symLink);
    ::IoDeleteDevice(DriverObject->DeviceObject);
    ok("Device '%S' deleted", DEVICE_NAME);

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
    PPRIVILEGE_SET lpRequiredPrivileges                                                            = nullptr;
    UCHAR ucPrivilegesBuffer[FIELD_OFFSET(PRIVILEGE_SET, Privilege) + sizeof(LUID_AND_ATTRIBUTES)] = {0};

    lpRequiredPrivileges                             = (PPRIVILEGE_SET)ucPrivilegesBuffer;
    lpRequiredPrivileges->PrivilegeCount             = 1;
    lpRequiredPrivileges->Control                    = PRIVILEGE_SET_ALL_NECESSARY;
    lpRequiredPrivileges->Privilege[0].Luid.LowPart  = SE_DEBUG_PRIVILEGE;
    lpRequiredPrivileges->Privilege[0].Luid.HighPart = 0;
    lpRequiredPrivileges->Privilege[0].Attributes    = 0;

    if ( ::SePrivilegeCheck(
             lpRequiredPrivileges,
             &lpSecurityContext->AccessState->SubjectSecurityContext,
             Irp->RequestorMode) == false )
    {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }
    else
    {
        auto scoped_lock          = CFB::Driver::Utils::ScopedLock(Globals->ContextLock);
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

    const ULONG IoctlCode       = CurrentStack->Parameters.DeviceIoControl.IoControlCode;
    PVOID InputBuffer           = Irp->AssociatedIrp.SystemBuffer;
    const ULONG InputBufferLen  = min(CurrentStack->Parameters.DeviceIoControl.InputBufferLength, CFB_DRIVER_MAX_PATH);
    PVOID OutputBuffer          = Irp->AssociatedIrp.SystemBuffer;
    const ULONG OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG dwDataWritten         = 0;

#ifdef _DEBUG
    CFB::Utils::Hexdump(InputBuffer, InputBufferLen);
#endif // _DEBUG

    dbg("Attempting to process IOCTL %#x (IRQL=%d)", IoctlCode, ::KeGetCurrentIrql());


    switch ( IoctlCode )
    {
    case IOCTL_HookDriver:
    {
        const wchar_t* DriverName = reinterpret_cast<wchar_t*>(InputBuffer);
        Status                    = Globals->DriverManager.InsertDriver(DriverName);
        break;
    }

    case IOCTL_UnhookDriver:
    {
        const wchar_t* DriverName = reinterpret_cast<wchar_t*>(InputBuffer);
        Status                    = Globals->DriverManager.RemoveDriver(DriverName);
        break;
    }

    case IOCTL_GetNumberOfDrivers:
    {
        dwDataWritten = Globals->DriverManager.Items().Size();
        break;
    }

    case IOCTL_EnableMonitoring:
    {
        const wchar_t* DriverName = reinterpret_cast<wchar_t*>(InputBuffer);
        Status                    = Globals->DriverManager.SetMonitoringState(DriverName, true);
        break;
    }

    case IOCTL_DisableMonitoring:
    {
        const wchar_t* DriverName = reinterpret_cast<wchar_t*>(InputBuffer);
        Status                    = Globals->DriverManager.SetMonitoringState(DriverName, false);
        break;
    }

    case IOCTL_SetEventPointer:
    {
        const HANDLE hEvent = *((PHANDLE)InputBuffer);
        Status              = Globals->IrpManager.SetEvent(hEvent);
        break;
    }

    case IOCTL_GetDriverInfo:
    case IOCTL_EnableDriver:
    case IOCTL_DisableDriver:

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

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PIO_STACK_LOCATION pStack = ::IoGetCurrentIrpStackLocation(Irp);
    if ( pStack == nullptr )
    {
        err("IoGetCurrentIrpStackLocation() failed (IRP %p)", Irp);
        return CompleteRequest(Irp, Status, 0);
    }

    const u32 RequestedBufferSize = pStack->Parameters.Read.Length;
    usize ExpectedBufferSize      = 0;

    {
        Globals->IrpManager.Items().ForEach(
            [&ExpectedBufferSize](CFB::Driver::CapturedIrp* Irp)
            {
                ExpectedBufferSize += Irp->DataSize();
                return true;
            });
    }

    //
    // If the buffer size is 0, return the expected size
    //
    if ( RequestedBufferSize == 0 )
    {
        return CompleteRequest(Irp, STATUS_SUCCESS, ExpectedBufferSize);
    }

    //
    // Otherwise, check the arguments and write the IRPs to the client
    //
    if ( RequestedBufferSize != ExpectedBufferSize )
    {
        return CompleteRequest(Irp, STATUS_INFO_LENGTH_MISMATCH, ExpectedBufferSize);
    }

    NT_ASSERT(Irp->MdlAddress);

    PVOID Buffer = ::MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
    if ( Buffer == nullptr )
    {
        return CompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }

    /*

    Status = PopFromQueue(&pInterceptedIrp);
    if ( !NT_SUCCESS(Status) )
    {
        return CompleteRequest(Irp, Status, 0);
    }

    //
    // Copy the header (always)
    //
    PINTERCEPTED_IRP_HEADER pInterceptedIrpHeader = pInterceptedIrp->Header;
    ::RtlCopyMemory( Buffer, pInterceptedIrpHeader, sizeof(INTERCEPTED_IRP_HEADER) );
    BufferOffset += sizeof(INTERCEPTED_IRP_HEADER);


    //
    // Copy the IRP input buffer (if any)
    //
    if(pInterceptedIrpHeader->InputBufferLength && pInterceptedIrp->InputBuffer)
    {
        ULONG_PTR RawBuffer = ((ULONG_PTR)Buffer) + BufferOffset;
        ::RtlCopyMemory((PVOID)RawBuffer, pInterceptedIrp->InputBuffer, pInterceptedIrpHeader->InputBufferLength);
        BufferOffset += pInterceptedIrpHeader->InputBufferLength;
    }

    //
    // Copy the IRP output buffer (if any)
    //
    if (pInterceptedIrpHeader->OutputBufferLength && pInterceptedIrp->OutputBuffer)
    {
        ULONG_PTR RawBuffer = ((ULONG_PTR)Buffer) + BufferOffset;
        ::RtlCopyMemory((PVOID)RawBuffer, pInterceptedIrp->OutputBuffer, pInterceptedIrpHeader->OutputBufferLength);
        BufferOffset += pInterceptedIrpHeader->OutputBufferLength;
    }

    FreeInterceptedIrp(pInterceptedIrp);
    */

    return CompleteRequest(Irp, STATUS_SUCCESS, 0);
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

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    do
    {
        info("Initializing global context...");
        Globals = new GlobalContext();

        info("Loading device '%S'...", DEVICE_NAME);
        PDEVICE_OBJECT DeviceObject = nullptr;
        UNICODE_STRING name         = RTL_CONSTANT_STRING(DEVICE_PATH);
        UNICODE_STRING symLink      = RTL_CONSTANT_STRING(DOS_DEVICE_PATH);

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
            &name,
            FILE_DEVICE_UNKNOWN,
            FILE_DEVICE_SECURE_OPEN,
            false, // ACL is managed by DriverCreateRoutine
            &DeviceObject);
        if ( !NT_SUCCESS(Status) )
        {
            err("Error creating device object (0x%08X)", Status);
            break;
        }

        ok("Device '%S' successfully created", DEVICE_NAME);

        Status = ::IoCreateSymbolicLink(&symLink, &name);
        if ( !NT_SUCCESS(Status) )
        {
            err("IoCreateSymbolicLink() failed: 0x%08X", Status);
            break;
        }

        ok("Symlink for '%S' created", DEVICE_NAME);

        DeviceObject->Flags |= DO_DIRECT_IO;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        Globals->DeviceObject = DeviceObject;
        Globals->DriverObject = DriverObject;

        ok("Device initialization  for '%S' done", DEVICE_NAME);

    } while ( false );

    if ( !NT_SUCCESS(Status) )
    {
        if ( Globals )
        {
            if ( Globals->DeviceObject )
            {
                ::IoDeleteDevice(Globals->DeviceObject);
            }

            delete Globals;
        }
    }

    return Status;
}
