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
    auto CapturedIrp = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::Irp, DeviceObject);
    if ( !(CapturedIrp && CapturedIrp->IsValid()) )
    {
        //
        // Call the original routine
        //
        PDRIVER_DISPATCH OriginalIoctlDeviceControl = DeviceObject->DriverObject->MajorFunction[Stack->MajorFunction];
        return OriginalIoctlDeviceControl(DeviceObject, Irp);
    }

    auto const Driver = CapturedIrp->AssociatedDriver();
    NTSTATUS Status   = STATUS_SUCCESS;

    dbg("Initialized CapturedIrp at %p", CapturedIrp);

    // TODO: replace with SharedPointer<CapturedIrp>

    if ( Driver->CanCapture() )
    {
        dbg("Capturing pre data of IRP to '%wZ'", Driver->Path.get());
        Status = CapturedIrp->CapturePreCallData(Irp);
        if ( !NT_SUCCESS(Status) )
        {
            warn("CapturedIrp->CapturePreCallData(%p) failed with Status = 0x%08X", CapturedIrp, Status);
            Driver->DisableCapturing();
        }
    }

    //
    // Call the original routine
    //
    PDRIVER_DISPATCH OriginalIoctlDeviceControl = Driver->OriginalDriverObject->MajorFunction[Stack->MajorFunction];
    NTSTATUS IoctlStatus                        = OriginalIoctlDeviceControl(DeviceObject, Irp);

    //
    // Collect the output from the call
    //
    if ( Driver->CanCapture() )
    {
        dbg("Capturing post data of IRP to '%wZ'", Driver->Path.get());
        Status = CapturedIrp->CapturePostCallData(Irp, IoctlStatus);
        if ( NT_SUCCESS(Status) )
        {
            if ( Globals->IrpManager.Push(CapturedIrp) )
            {
                //
                // If everything went fine, return the result
                //
                Driver->IncrementIrpCount();
                return IoctlStatus;
            }

            warn("Failed to push new IRP %p to the queue (Status = 0x%08X)", CapturedIrp, Status);
        }
        else
        {
            warn("CapturedIrp->CapturePostCallData(%p) failed with Status = 0x%08X", CapturedIrp, Status);
        }

        //
        // If here, there's been a problem, disable capturing
        //
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


///
/// @brief The `InterceptGenericFastIoDeviceControl()` interception routine wrapper.
///
/// @param FileObject
/// @param Wait
/// @param InputBuffer
/// @param InputBufferLength
/// @param OutputBuffer
/// @param OutputBufferLength
/// @param IoControlCode
/// @param IoStatus
/// @param DeviceObject
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoDeviceControl(
    _In_ PFILE_OBJECT FileObject,
    _In_ BOOLEAN Wait,
    _In_opt_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_opt_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _In_ ULONG IoControlCode,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject)
{
    //
    // Prepare the object
    //
    auto CapturedFastIo = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::FastIo_Ioctl, DeviceObject);
    if ( !(CapturedFastIo && CapturedFastIo->IsValid()) )
    {
        //
        // Call the original routine
        //
        const PDRIVER_OBJECT Driver                               = FileObject->DeviceObject->DriverObject;
        const PFAST_IO_DEVICE_CONTROL OriginalFastIoDeviceControl = Driver->FastIoDispatch->FastIoDeviceControl;
        return OriginalFastIoDeviceControl(
            FileObject,
            Wait,
            InputBuffer,
            InputBufferLength,
            OutputBuffer,
            OutputBufferLength,
            IoControlCode,
            IoStatus,
            DeviceObject);
    }

    const auto Driver       = CapturedFastIo->AssociatedDriver();
    const bool bCaptureData = Driver->CanCapture();

    //
    // If capturing enabled, capture the input data
    //
    if ( bCaptureData )
    {
        CapturedFastIo->CapturePreCallFastIoData(InputBuffer, IoControlCode);
    }

    //
    // Execute the original callback
    //
    PFAST_IO_DEVICE_CONTROL OriginalFastIoDeviceControl =
        Driver->OriginalDriverObject->FastIoDispatch->FastIoDeviceControl;
    BOOLEAN bRes = OriginalFastIoDeviceControl(
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
    // If capturing enabled, capture the output data
    //
    if ( bCaptureData )
    {
        CapturedFastIo->CapturePostCallFastIoData(OutputBuffer);

        //
        // And push the IRP to the queue
        //
        Globals->IrpManager.Push(CapturedFastIo);
        Driver->IncrementIrpCount();
    }
    else
    {
        //
        // Otherwise just delete the allocation
        //
        delete CapturedFastIo;
    }

    return bRes;
}


///
/// @brief The `InterceptGenericFastIoRead()` interception routine wrapper.
///
/// @param FileObject
/// @param FileOffset
/// @param BufferLength
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
///
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoRead(
    _In_ PFILE_OBJECT FileObject,
    _In_ PLARGE_INTEGER FileOffset,
    _In_ ULONG BufferLength,
    _In_ BOOLEAN Wait,
    _In_ ULONG LockKey,
    _Out_ PVOID Buffer,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject)
{
    //
    // Prepare the object
    //
    auto CapturedFastRead = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::FastIo_Ioctl, DeviceObject);
    if ( !(CapturedFastRead && CapturedFastRead->IsValid()) )
    {
        //
        // Call the original routine
        //
        const PDRIVER_OBJECT Driver            = FileObject->DeviceObject->DriverObject;
        const PFAST_IO_READ OriginalFastIoRead = Driver->FastIoDispatch->FastIoRead;
        return OriginalFastIoRead(FileObject, FileOffset, BufferLength, Wait, LockKey, Buffer, IoStatus, DeviceObject);
    }

    auto Driver             = CapturedFastRead->AssociatedDriver();
    const bool bCaptureData = Driver->CanCapture();

    dbg("Initialized CapturedFastRead at %p", CapturedFastRead);
    //
    // Execute the original callback
    //
    PFAST_IO_READ const OriginalFastIoRead = Driver->OriginalDriverObject->FastIoDispatch->FastIoRead;
    BOOLEAN bRes =
        OriginalFastIoRead(FileObject, FileOffset, BufferLength, Wait, LockKey, Buffer, IoStatus, DeviceObject);

    //
    // If capturing enabled, capture the output data
    //
    if ( bCaptureData )
    {
        CapturedFastRead->CapturePostCallFastIoData(Buffer);

        //
        // And push the IRP to the queue
        //
        Globals->IrpManager.Push(CapturedFastRead);
        Driver->IncrementIrpCount();
    }
    else
    {
        //
        // Otherwise just delete the allocation
        //
        delete CapturedFastRead;
    }

    return bRes;
}


///
/// @brief The InterceptGenericFastIoWrite() interception routine wrapper.
///
/// @param FileObject
/// @param FileOffset
/// @param Length
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
///
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoWrite(
    _In_ PFILE_OBJECT FileObject,
    _In_ PLARGE_INTEGER FileOffset,
    _In_ ULONG BufferLength,
    _In_ BOOLEAN Wait,
    _In_ ULONG LockKey,
    _Out_ PVOID Buffer,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject)
{
    //
    // Prepare the object
    //
    auto CapturedFastWrite =
        new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::FastIo_Ioctl, DeviceObject);
    if ( !(CapturedFastWrite && CapturedFastWrite->IsValid()) )
    {
        //
        // Call the original routine
        //
        const PDRIVER_OBJECT Driver             = FileObject->DeviceObject->DriverObject;
        const PFAST_IO_READ OriginalFastIoWrite = Driver->FastIoDispatch->FastIoWrite;
        return OriginalFastIoWrite(FileObject, FileOffset, BufferLength, Wait, LockKey, Buffer, IoStatus, DeviceObject);
    }

    auto Driver             = CapturedFastWrite->AssociatedDriver();
    const bool bCaptureData = Driver->CanCapture();

    dbg("Initialized CapturedFastWrite at %p", CapturedFastWrite);
    //
    // If capturing enabled, capture the input data
    //
    if ( bCaptureData )
    {
        CapturedFastWrite->CapturePreCallFastIoData(Buffer, 0);
    }

    //
    // Execute the original callback
    //
    PFAST_IO_READ const OriginalFastIoWrite = Driver->OriginalDriverObject->FastIoDispatch->FastIoWrite;
    BOOLEAN bRes =
        OriginalFastIoWrite(FileObject, FileOffset, BufferLength, Wait, LockKey, Buffer, IoStatus, DeviceObject);

    //
    // If capturing enabled, capture the output data
    //
    if ( bCaptureData )
    {
        //
        // And push the IRP to the queue
        //
        Globals->IrpManager.Push(CapturedFastWrite);
        Driver->IncrementIrpCount();
    }
    else
    {
        //
        // Otherwise just delete the allocation
        //
        delete CapturedFastWrite;
    }

    return bRes;
}
} // namespace CFB::Driver::Callbacks
