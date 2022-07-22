#include "Callbacks.hpp"

#include "Context.hpp"
#include "HookedDriver.hpp"

namespace CFB::Driver::Callbacks
{


NTSTATUS
static inline CompleteRequest(_In_ PIRP Irp, _In_ NTSTATUS Status, _In_ ULONG_PTR Information)
{
    Irp->IoStatus.Status      = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

///
/// @brief This is the main interception routine: it will find the HookedDriver  associated to a DeviceObject. If
/// any is found, and capture mode is enabled the IRP data will be pushed to the queue of captured data.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
NTSTATUS
InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
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

    HookedDriver* const Driver = Globals->DriverManager.Entries.Find(FilterByDeviceAddress);
    if ( Driver == nullptr )
    {
        //
        // This is really bad: if here the interception routine got called by a non-hooked driver
        // Could be a bad pointer restoration. Anyway, we log and fail for now.
        //
        err("Failed to find a HookedDriver object associated to the received IRP.\n"
            "This could indicates a corruption of the hooked driver list, you should probably reboot...\n");
        return CompleteRequest(Irp, STATUS_NO_SUCH_DEVICE, 0);
    }


    //
    // Capture the IRP data if enabled
    //
    if ( Driver->Enabled )
    {
        auto CapturedIrp = new CFB::Driver::CapturedIrp(Driver, DeviceObject, Irp);

        // TODO: push to queue

        delete CapturedIrp;

        Driver->InterceptedIrpsCount++;
    }

    /*
    PINTERCEPTED_IRP pIrpInfo = NULL;
    if ( IsMonitoringEnabled() && curDriver->Enabled == TRUE && pCurrentOwnerProcess != PsGetCurrentProcess() )
    {
        NTSTATUS Status = HandleInterceptedIrp(curDriver, DeviceObject, Irp, &pIrpInfo);

        if ( !NT_SUCCESS(Status) && Status != STATUS_NOT_IMPLEMENTED )
        {
            CfbDbgPrintWarn(L"HandleInterceptedIrp() failed (status=0x%X)\n", Status);
        }
        else
        {
            curDriver->NumberOfRequestIntercepted++;
        }
    }
    */


    //
    // And call the original routine
    //
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);
    PVOID UserBuffer         = nullptr;

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
        break;
    }

    PDRIVER_DISPATCH OriginalIoctlDeviceControl = Driver->OriginalRoutines[Stack->MajorFunction];
    NTSTATUS IoctlStatus                        = OriginalIoctlDeviceControl(DeviceObject, Irp);

    //
    // Collect the result from the result
    //

    /*
    if ( pCurrentOwnerProcess != PsGetCurrentProcess() && IsMonitoringEnabled() && curDriver->Enabled == TRUE &&
         pIrpInfo != NULL )
    {
        NTSTATUS Status = CompleteHandleInterceptedIrp(UserBuffer, IoctlStatus, pIrpInfo);
        if ( !NT_SUCCESS(Status) )
            CfbDbgPrintWarn(L"CompleteHandleInterceptedIrp() failed, Status=0x%x\n", Status);
    }

    //
    // Push the new irp to the queue and notify the broker
    //
    if ( pIrpInfo )
    {
        DWORD dwType = pIrpInfo->Header->Type;
        DWORD dwSize;
        NTSTATUS Status = PushToQueue(pIrpInfo);
        if ( !NT_SUCCESS(Status) )
        {
            CfbDbgPrintErr(L"PushToQueue(%p) failed, status=0x%x\n", pIrpInfo, Status);
            FreeInterceptedIrp(pIrpInfo);
        }
        else
        {
            dwSize = GetIrpListSize() ? GetIrpListSize() - 1 : 0;
            CfbDbgPrintOk(L"pushing IRPs[%d] = %p (type=%d)\n", dwSize, pIrpInfo, dwType);
            NotifyUserNewEvent();
        }
    }

    return IoctlStatus;
    */

    return STATUS_SUCCESS;
}

NTSTATUS
InterceptedDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


///
/// @brief The ReadFile() interception routine wrapper.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS STATUS_SUCCESS on success.
///
NTSTATUS
InterceptedReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


///
/// @brief The WriteFile() interception routine wrapper.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
NTSTATUS
InterceptedWriteRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


///
/// @brief The `InterceptGenericFastIoRoutinePre()` routine wrapper intercepts FastIOs input data.
///
/// @param DeviceObject
/// @param Type
/// @param Buffer
/// @param BufferLength
/// @param IoControlCode
/// @param Flags
/// @param pIrpOut
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoRoutine(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ UINT32 Type,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _In_ ULONG IoControlCode,
    _In_ UINT32 Flags,
    _Inout_ PVOID* pIrpOut)
{
    auto ByDeviceAddress = [&DeviceObject](const HookedDriver* h)
    {
        for ( PDEVICE_OBJECT CurrentDevice = h->DriverObject->DeviceObject; CurrentDevice;
              CurrentDevice                = CurrentDevice->NextDevice )
        {
            if ( CurrentDevice == DeviceObject )
                return true;
        }
        return false;
    };

    HookedDriver* const Driver = Globals->DriverManager.Entries.Find(ByDeviceAddress);
    if ( Driver == nullptr )
    {
        err("Failed to find driver for InterceptGenericFastIoRoutine(). "
            "This could mean a corrupted state of '%s'."
            "You should reboot to avoid further corruption...\n",
            CFB_DEVICE_NAME);
        return FALSE;
    }

    NTSTATUS Status = STATUS_SUCCESS;

    /*
    if ( Driver->Enabled )
    {
        Status =
            HandleInterceptedFastIo(Driver, DeviceObject, Type, IoControlCode, Buffer, BufferLength, Flags,
    &*pIrpOut);
    }
    */

    return Status;
}


///
/// @brief The `InterceptGenericFastIoDeviceControl()` interception routine wrapper.
///
/// ```c typedef BOOLEAN (*PFAST_IO_DEVICE_CONTROL)(
///     IN struct _FILE_OBJECT* FileObject,
///     IN BOOLEAN Wait,
///     IN PVOID InputBuffer OPTIONAL,
///     IN ULONG InputBufferLength,
///     OUT PVOID OutputBuffer OPTIONAL,
///     IN ULONG OutputBufferLength,
///     IN ULONG IoControlCode,
///     OUT PIO_STATUS_BLOCK IoStatus,
///     IN struct _DEVICE_OBJECT* DeviceObject);
/// ```
///
/// @param FileObject
/// @param Wait
/// @param OPTIONAL
/// @param InputBufferLength
/// @param OPTIONAL
/// @param OutputBufferLength
/// @param IoControlCode
/// @param IoStatus
/// @param DeviceObject
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoDeviceControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    /*
    PINTERCEPTED_IRP pIrp = NULL;

    //
    // Capture the input
    //
    if ( !InterceptGenericFastIoRoutine(
             DeviceObject,
             CFB_INTERCEPTED_IRP_TYPE_FASTIO_IOCTL,
             InputBuffer,
             InputBufferLength,
             IoControlCode,
             CFB_FASTIO_USE_INPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
             &pIrp) )
        return FALSE;

    PHOOKED_DRIVER Driver                               = GetHookedDriverFromDeviceObject(DeviceObject);
    PFAST_IO_DEVICE_CONTROL OriginalFastIoDeviceControl = Driver->FastIoDeviceControl;
    BOOLEAN bRes                                        = OriginalFastIoDeviceControl(
        FileObject,
        Wait,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        IoControlCode,
        IoStatus,
        DeviceObject);

    //
    // Capture the output - pIrp was already initialized
    //
    if ( !InterceptGenericFastIoRoutine(
             DeviceObject,
             CFB_INTERCEPTED_IRP_TYPE_FASTIO_IOCTL,
             OutputBuffer,
             OutputBufferLength,
             IoControlCode,
             CFB_FASTIO_USE_OUTPUT_BUFFER,
             &pIrp) )
        return FALSE;

    return bRes;
    */
    return STATUS_SUCCESS;
}


///
/// @brief The `InterceptGenericFastIoRead()` interception routine wrapper.
///
///
/// ```c typedef BOOLEAN (*PFAST_IO_READ)(
///     IN PFILE_OBJECT FileObject,
///     IN PLARGE_INTEGER FileOffset,
///     IN ULONG Length,
///     IN BOOLEAN Wait,
///     IN ULONG LockKey,
///     OUT PVOID Buffer,
///     OUT PIO_STATUS_BLOCK IoStatus,
///     IN PDEVICE_OBJECT DeviceObject);
/// ```
///
/// @param FileObject
/// @param FileOffset
/// @param Length
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    /*
        PINTERCEPTED_IRP pIrp = NULL;
        PHOOKED_DRIVER Driver = GetHookedDriverFromDeviceObject(DeviceObject);
        if ( !Driver )
            return FALSE;

        PFAST_IO_READ OriginalFastIoRead = Driver->FastIoRead;
        BOOLEAN bRes = OriginalFastIoRead(FileObject, FileOffset, Length, Wait, LockKey, Buffer, IoStatus,
       DeviceObject);


        if ( !InterceptGenericFastIoRoutine(
                 DeviceObject,
                 CFB_INTERCEPTED_IRP_TYPE_FASTIO_READ,
                 Buffer,
                 Length,
                 (ULONG)-1,
                 CFB_FASTIO_USE_OUTPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
                 &pIrp) )
            return FALSE;

        return bRes;
    */

    return STATUS_SUCCESS;
}


///
/// @brief The InterceptGenericFastIoWrite() interception routine wrapper.
///
/// ```c typedef BOOLEAN (*PFAST_IO_WRITE)(
///     IN PFILE_OBJECT FileObject,
///     IN PLARGE_INTEGER FileOffset,
///     IN ULONG Length,
///     IN BOOLEAN Wait,
///     IN ULONG LockKey,
///     OUT PVOID Buffer,
///     OUT PIO_STATUS_BLOCK IoStatus,
///     IN PDEVICE_OBJECT DeviceObject);
/// ```
///
/// @param FileObject
/// @param FileOffset
/// @param Length
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    /*
    PINTERCEPTED_IRP pIrp = NULL;
    if ( !InterceptGenericFastIoRoutine(
             DeviceObject,
             CFB_INTERCEPTED_IRP_TYPE_FASTIO_WRITE,
             Buffer,
             Length,
             (ULONG)-1,
             CFB_FASTIO_USE_INPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
             &pIrp) )
        return FALSE;

    PHOOKED_DRIVER Driver              = GetHookedDriverFromDeviceObject(DeviceObject);
    PFAST_IO_WRITE OriginalFastIoWrite = Driver->FastIoWrite;
    return OriginalFastIoWrite(FileObject, FileOffset, Length, Wait, LockKey, Buffer, IoStatus, DeviceObject);
*/
    return STATUS_SUCCESS;
}
} // namespace CFB::Driver::Callbacks
