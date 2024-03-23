#define CFB_NS "[CFB::Driver::Callbacks]"

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

NTSTATUS static ExecuteOriginalCallback(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp,
    _Out_ NTSTATUS* IoctlStatus)
{
    *IoctlStatus             = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    __try
    {
        PDRIVER_DISPATCH OriginalIoctlDeviceControl = DriverObject->MajorFunction[Stack->MajorFunction];
        *IoctlStatus                                = OriginalIoctlDeviceControl(DeviceObject, Irp);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        NTSTATUS Status = GetExceptionCode();
        crit("Exception 0x%08x caught while executing original function", Status);
        return Status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    NTSTATUS Status          = STATUS_UNSUCCESSFUL;
    NTSTATUS IoctlStatus     = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    //
    // Capture the IRP data if capturing mode is enabled for the current driver
    //
    auto CapturedIrp = new CFB::Driver::CapturedIrp(CFB::Driver::CapturedIrp::IrpType::Irp, DeviceObject);
    if ( !CapturedIrp->IsValid() )
    {
        //
        // There was an issue at the allocation or initialization of the CapturedIrp object. Fallback safely by calling
        // the original routine
        //
        delete CapturedIrp;
        warn("Failed to initialize CapturedIrp(), fallback");
        PDRIVER_DISPATCH OriginalIoctlDeviceControl = DeviceObject->DriverObject->MajorFunction[Stack->MajorFunction];
        return OriginalIoctlDeviceControl(DeviceObject, Irp);
    }

    auto const Driver = CapturedIrp->AssociatedDriver();

    //
    // Consequently, `Driver` here should never be null
    //
    NT_ASSERT(Driver != nullptr);


    dbg("Initialized CapturedIrp at %p", CapturedIrp);

    // TODO: replace with SharedPointer<CapturedIrp>

    if ( Driver->CanCapture() )
    {
        dbg("Capturing pre data of IRP to '%wZ'", Driver->Path.get());
        Status = CapturedIrp->CapturePreCallData(Irp);
        if ( !NT_SUCCESS(Status) )
        {
            warn("CapturedIrp(%p)->CapturePreCallData(IRP=%p) failed with Status = 0x%08X", CapturedIrp, Irp, Status);
            Driver->DisableCapturing();
        }
    }

    //
    // Call the original routine
    //
    Status = ExecuteOriginalCallback(Driver->OriginalDriverObject, DeviceObject, Irp, &IoctlStatus);
    if ( !NT_SUCCESS(Status) )
    {
        //
        // The status of `ExecuteOriginalCallback` indicates whether  an exception was hit. This can happen if the
        // driver was unloaded. If so disable the HookDriver object and mark it invalid to never touch the invalid
        // memory area again.
        //
        warn("ExecuteOriginalCallback() failed with Status = Status = 0x%08X", Status);
        Driver->DisableCapturing();
        Driver->FlagAsInvalid();
    }

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


NTSTATUS
InterceptedDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


NTSTATUS
InterceptedReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


NTSTATUS
InterceptedWriteRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    return InterceptGenericRoutine(DeviceObject, Irp);
}


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
