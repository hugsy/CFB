#include "Callbacks.hpp"

#include "CapturedIrp.hpp"
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
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    //
    // Capture the IRP data if capturing mode is enabled for the current driver
    //
    auto CapturedIrp              = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::Irp, DeviceObject);
    HookedDriver* const Driver    = CapturedIrp->AssociatedDriver();
    const bool IsCapturingEnabled = Driver->HasCapturingEnabled();

    // TODO: replace with SharedPointer(CapturedIrp)

    if ( IsCapturingEnabled )
    {
        CapturedIrp->CapturePreCallData(Irp);
    }

    //
    // Call the original routine
    //
    PDRIVER_DISPATCH OriginalIoctlDeviceControl = Driver->OriginalRoutines[Stack->MajorFunction];
    NTSTATUS IoctlStatus                        = OriginalIoctlDeviceControl(DeviceObject, Irp);

    //
    // Collect the result from the result
    //
    if ( IsCapturingEnabled )
    {
        if ( NT_SUCCESS(CapturedIrp->CapturePostCallData(Irp, IoctlStatus)) &&
             true == Globals->IrpCollector.Push(CapturedIrp) )
        {
            Driver->IncrementIrpCount();
            return IoctlStatus;
        }

        //
        // Pushing failed, there might a problem with the queue. Disable interception to
        // avoid more damage
        //
        warn("Failed to push new IRP %p to the queue", CapturedIrp);
        Driver->DisableCapturing();
    }

    delete CapturedIrp;
    return IoctlStatus;
}


///
/// @brief
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
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


/**

NTSTATUS
HandleInterceptedFastIo(
    _In_ PHOOKED_DRIVER Driver,
    _In_ PDEVICE_OBJECT pDeviceObject,
    _In_ UINT32 Type,
    _In_ UINT32 IoctlCode,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _In_ UINT32 Flags,
    _Inout_ PINTERCEPTED_IRP* pIrpOut
)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PINTERCEPTED_IRP pIrp = NULL;
    HOOKED_IRP_INFO temp = { 0, };

    BOOLEAN IsInput = Flags & CFB_FASTIO_USE_INPUT_BUFFER;


    //
    // prepare the metadata
    //
    temp.Pid = HandleToULong(PsGetProcessId(PsGetCurrentProcess()));
    temp.Tid = HandleToULong(PsGetCurrentThreadId());
    temp.Type = Type;
    temp.IoctlCode = IoctlCode;

    wcsncpy_s(temp.DriverName, MAX_PATH*sizeof(WCHAR), Driver->Name, _TRUNCATE);

    Status = GetDeviceNameFromDeviceObject(pDeviceObject, temp.DeviceName, MAX_PATH);
    if (!NT_SUCCESS(Status))
        CfbDbgPrintWarn(L"Cannot get device name, using empty string (Status=0x%#x)\n", Status);


    //
    // Copy the input buffer
    //
    if (IsInput)
    {
        if (BufferLength && Buffer)
        {
            temp.InputBuffer = ExAllocatePoolWithTag(
                NonPagedPool,
                BufferLength,
                CFB_DEVICE_TAG
            );
            if (!temp.InputBuffer)
                return STATUS_INSUFFICIENT_RESOURCES;

            RtlSecureZeroMemory(temp.InputBuffer, BufferLength);

            RtlCopyMemory(temp.InputBuffer, Buffer, BufferLength);

            temp.InputBufferLen = BufferLength;
        }
        else
        {
            temp.InputBufferLen = 0;
            temp.InputBuffer = NULL;
        }
    }
    else
    {
        if (BufferLength && Buffer)
        {
            temp.OutputBuffer = ExAllocatePoolWithTag(
                NonPagedPool,
                BufferLength,
                CFB_DEVICE_TAG
            );
            if (!temp.OutputBuffer)
                return STATUS_INSUFFICIENT_RESOURCES;

            RtlSecureZeroMemory(temp.OutputBuffer, BufferLength);

            RtlCopyMemory(temp.OutputBuffer, Buffer, BufferLength);

            temp.OutputBufferLen = BufferLength;
        }
        else
        {
            temp.OutputBufferLen = 0;
            temp.OutputBuffer = NULL;
        }
    }



    if (Flags & CFB_FASTIO_INIT_QUEUE_MESSAGE)
    {
        //
        // Prepare the message to be queued
        //
        Status = PreparePipeMessage(&temp, &pIrp);

        if (!NT_SUCCESS(Status) || pIrp == NULL)
        {
            CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
            if (IsInput==TRUE && temp.InputBuffer)
            {
                ExFreePoolWithTag(temp.InputBuffer, CFB_DEVICE_TAG);
                temp.InputBuffer = NULL;
            }
            else if (IsInput==FALSE && temp.OutputBuffer)
            {
                ExFreePoolWithTag(temp.OutputBuffer, CFB_DEVICE_TAG);
                temp.OutputBuffer = NULL;
            }

            return Status;
        }
    }

    *pIrpOut = pIrp;
    return Status;
}
 */

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
    _In_ CFB::Driver::CapturedIrp::IrpType Type,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _In_ ULONG IoControlCode,
    _In_ UINT32 Flags,
    _Inout_ PVOID* pIrpOut)
{
    NTSTATUS Status                      = STATUS_SUCCESS;
    CFB::Driver::CapturedIrp* CapturedIo = nullptr;

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

    //
    // If capture is enabled, create a FastIo object, capture the input buffer (if appropriate)
    //
    if ( Driver->HasCapturingEnabled() )
    {
        CapturedIo = new CFB::Driver::CapturedIrp(Type, DeviceObject);
        if ( CapturedIo )
        {
            delete CapturedIo;
        }
    }


    //
    // Execute the original callback
    //


    //
    // Capture the output buffer (if appropriate)
    //
    if ( Driver->HasCapturingEnabled() )
    {
        if ( CapturedIo )
        {
        }
    }

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
             CFB::Driver::CapturedIrp::Type::FastIo_Ioctl,
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
