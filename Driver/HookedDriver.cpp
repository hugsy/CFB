#include "HookedDriver.hpp"

#include "Native.hpp"

namespace Callbacks = CFB::Driver::Callbacks;
namespace Utils     = CFB::Driver::Utils;

namespace CFB::Driver
{
HookedDriver::HookedDriver(const PUNICODE_STRING UnicodePath) :
    Enabled(false),
    State(HookState::Unhooked),
    OriginalDriverObject(nullptr),
    Next(),
    // OriginalRoutines(),
    // FastIoRead(nullptr),
    // FastIoWrite(nullptr),
    // FastIoDeviceControl(nullptr),
    InterceptedIrpsCount(0),
    Path(UnicodePath->Buffer, UnicodePath->Length, NonPagedPoolNx),
    HookedDriverObject(new (NonPagedPoolNx) DRIVER_OBJECT)
{
    dbg("Creating HookedDriver('%wZ')", &Path);

    //
    // Find the driver from its path
    //
    NTSTATUS Status = ::ObReferenceObjectByName(
        Path.get(),
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        nullptr,
        (PVOID*)&OriginalDriverObject);

    NT_ASSERT(NT_SUCCESS(Status));
    NT_ASSERT(OriginalDriverObject != nullptr);

    //
    // Create a copy DRIVER_OBJECT
    //
    ::RtlCopyMemory(HookedDriverObject.get(), OriginalDriverObject, sizeof(DRIVER_OBJECT));

    //
    // Swap the IRP major function callbacks of the HookedDriver, avoiding to trigger PatchGuard
    //
    SwapCallbacks();
}

HookedDriver::~HookedDriver()
{
    dbg("Destroying HookedDriver '%wZ'", Path.get());

    DisableCapturing();
    RestoreCallbacks();

    ObDereferenceObject(OriginalDriverObject);
}

void
HookedDriver::SwapCallbacks()
{
    Utils::ScopedLock lock(Mutex);

    if ( State != HookState::Unhooked )
    {
        warn("Invalid state: expecting 'Unhooked'");
        return;
    }

    //
    // Hook all `IRP_MJ_*`
    //
    for ( u16 i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ )
    {
        PDRIVER_DISPATCH OldRoutine = (PDRIVER_DISPATCH)InterlockedExchangePointer(
            // (PVOID*)&OriginalDriverObject->MajorFunction[i],
            (PVOID*)&HookedDriverObject->MajorFunction[i],
            (PVOID)Callbacks::InterceptGenericRoutine);

        // OriginalRoutines[i] = OldRoutine;
    }

    //
    // Hook FastIo too
    //
    if ( OriginalDriverObject->FastIoDispatch )
    {
        PFAST_IO_DEVICE_CONTROL OldFastIoDeviceControl = (PFAST_IO_DEVICE_CONTROL)InterlockedExchangePointer(
            // (PVOID*)&OriginalDriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID)Callbacks::InterceptGenericFastIoDeviceControl);

        // FastIoDeviceControl = OldFastIoDeviceControl;

        PFAST_IO_READ OldFastIoRead = (PFAST_IO_READ)InterlockedExchangePointer(
            // (PVOID*)&OriginalDriverObject->FastIoDispatch->FastIoRead,
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoRead,
            (PVOID)Callbacks::InterceptGenericFastIoRead);

        // FastIoRead = OldFastIoRead;

        PFAST_IO_WRITE OldFastIoWrite = (PFAST_IO_WRITE)InterlockedExchangePointer(
            // (PVOID*)&OriginalDriverObject->FastIoDispatch->FastIoWrite,
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoWrite,
            (PVOID)Callbacks::InterceptGenericFastIoWrite);

        // FastIoWrite = OldFastIoWrite;
    }

    //
    // This is where the hooking takes places: for each device object of the original driver, we replace
    // the driver object pointer member making it point to our hooked version. Doing so avoid triggering
    // PatchGuard and is more stealth.
    // TODO: test debug=off, testsigning=on
    //
    for ( PDEVICE_OBJECT DeviceObject = OriginalDriverObject->DeviceObject; DeviceObject != nullptr;
          DeviceObject                = DeviceObject->NextDevice )
    {
        DeviceObject->DriverObject = HookedDriverObject.get();
    }

    //
    // Switch state flag to hooked
    //
    State = HookState::Hooked;
}

void
HookedDriver::RestoreCallbacks()
{
    Utils::ScopedLock lock(Mutex);

    if ( State != HookState::Hooked )
    {
        warn("Invalid state: expecting 'Hooked'");
        return;
    }

    //
    // For each device object, restore the original driver object pointer
    //
    for ( PDEVICE_OBJECT DeviceObject = OriginalDriverObject->DeviceObject; DeviceObject != nullptr;
          DeviceObject                = DeviceObject->NextDevice )
    {
        DeviceObject->DriverObject = OriginalDriverObject;
    }

    /**
    //
    // Restore the original callbacks
    //

    for ( u16 i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ )
    {
        InterlockedExchangePointer((PVOID*)&OriginalDriverObject->MajorFunction[i], (PVOID)OriginalRoutines[i]);
    }

    //
    // Restore Fast IO Dispatch pointers
    //

    if ( OriginalDriverObject->FastIoDispatch )
    {
        InterlockedExchangePointer(
            (PVOID*)OriginalDriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID)FastIoDeviceControl);

        InterlockedExchangePointer((PVOID*)&OriginalDriverObject->FastIoDispatch->FastIoRead, (PVOID)FastIoRead);

        InterlockedExchangePointer((PVOID*)&OriginalDriverObject->FastIoDispatch->FastIoWrite, (PVOID)FastIoWrite);
    }
    */

    //
    // Switch back state flag
    //
    State = HookState::Unhooked;
}

bool
HookedDriver::EnableCapturing()
{
    Utils::ScopedLock lock(Mutex);
    bool bStatusChanged = !Enabled;
    Enabled             = true;
    return bStatusChanged;
}

bool
HookedDriver::DisableCapturing()
{
    Utils::ScopedLock lock(Mutex);
    bool bStatusChanged = Enabled;
    Enabled             = false;
    return bStatusChanged;
}


bool
HookedDriver::CanCapture()
{
    dbg("Driver('%wZ', State=%shooked, Enabled=%s)",
        Path.get(),
        (State == HookState::Unhooked) ? "un" : "",
        boolstr(Enabled));
    return State == HookState::Hooked && Enabled == true;
}


usize const
HookedDriver::IrpCount() const
{
    return InterceptedIrpsCount;
}


void
HookedDriver::IncrementIrpCount()
{
    InterlockedIncrement64((PLONG64)&InterceptedIrpsCount);
}


void
HookedDriver::DecrementIrpCount()
{
    InterlockedDecrement64((PLONG64)&InterceptedIrpsCount);
}


HookedDriver&
HookedDriver::operator++()
{
    IncrementIrpCount();
    return *this;
}


HookedDriver&
HookedDriver::operator--()
{
    DecrementIrpCount();
    return *this;
}


} // namespace CFB::Driver
