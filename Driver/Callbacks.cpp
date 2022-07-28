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
    auto CapturedIrp           = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::Irp, DeviceObject);
    HookedDriver* const Driver = CapturedIrp->AssociatedDriver();
    NTSTATUS Status            = STATUS_SUCCESS;

    // TODO: replace with SharedPointer<CapturedIrp>

    if ( Driver->HasCapturingEnabled() )
    {
        dbg("Capturing pre data of IRP to '%wZ'", Driver->Path.get());
        Status = CapturedIrp->CapturePreCallData(Irp);
        if ( !NT_SUCCESS(Status) )
        {
            warn("Failed to push new IRP %p to the queue (Status = 0x%08X)", CapturedIrp, Status);
            Driver->DisableCapturing();
        }
    }

    //
    // Call the original routine
    //
    PDRIVER_DISPATCH OriginalIoctlDeviceControl = Driver->OriginalRoutines[Stack->MajorFunction];
    NTSTATUS IoctlStatus                        = OriginalIoctlDeviceControl(DeviceObject, Irp);

    //
    // Collect the output from the call
    //
    dbg("Capturing post data of IRP to '%wZ'", Driver->Path.get());
    if ( Driver->HasCapturingEnabled() )
    {
        Status = CapturedIrp->CapturePostCallData(Irp, IoctlStatus);
        if ( !NT_SUCCESS(Status) || false == Globals->IrpCollector.Push(CapturedIrp) )
        {
            //
            // Something failed, there might a problem with the queue. Disable interception to
            // avoid more damage
            //
            warn("Failed to push new IRP %p to the queue (Status = 0x%08X)", CapturedIrp, Status);
            Driver->DisableCapturing();
        }
        else
        {
            //
            // If everything went fine, return the result
            //
            Driver->IncrementIrpCount();
            return IoctlStatus;
        }
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
    const auto Driver   = CapturedFastIo->AssociatedDriver();
    const bool bCaptureData = Driver->HasCapturingEnabled();

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
    // If capturing enabled, capture the output data
    //
    if ( bCaptureData )
    {
        CapturedFastIo->CapturePostCallFastIoData(OutputBuffer);

        //
        // And push the IRP to the queue
        //
        Globals->IrpCollector.Push(CapturedFastIo);
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
    auto Driver           = CapturedFastRead->AssociatedDriver();
    const bool bCaptureData = Driver->HasCapturingEnabled();

    //
    // Execute the original callback
    //
    PFAST_IO_READ const OriginalFastIoRead = Driver->FastIoRead;
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
        Globals->IrpCollector.Push(CapturedFastRead);
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
    auto CapturedFastRead = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::FastIo_Ioctl, DeviceObject);
    auto Driver           = CapturedFastRead->AssociatedDriver();
    const bool bCaptureData = Driver->HasCapturingEnabled();

    //
    // If capturing enabled, capture the input data
    //
    if ( bCaptureData )
    {
        CapturedFastRead->CapturePreCallFastIoData(Buffer, 0);
    }

    //
    // Execute the original callback
    //
    PFAST_IO_READ const OriginalFastIoRead = Driver->FastIoRead;
    BOOLEAN bRes =
        OriginalFastIoRead(FileObject, FileOffset, BufferLength, Wait, LockKey, Buffer, IoStatus, DeviceObject);

    //
    // If capturing enabled, capture the output data
    //
    if ( bCaptureData )
    {
        //
        // And push the IRP to the queue
        //
        Globals->IrpCollector.Push(CapturedFastRead);
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
} // namespace CFB::Driver::Callbacks
