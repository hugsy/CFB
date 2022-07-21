#include "HookedDriver.hpp"

#include "Native.hpp"

namespace Callbacks = CFB::Driver::Callbacks;
namespace Utils     = CFB::Driver::Utils;

namespace CFB::Driver
{
HookedDriver::HookedDriver(const PUNICODE_STRING UnicodePath) :
    Enabled(false),
    DriverObject(nullptr),
    Next(),
    OriginalRoutines(),
    FastIoRead(nullptr),
    FastIoWrite(nullptr),
    FastIoDeviceControl(nullptr),
    InterceptedIrpsCount(0),
    State(HookState::Unhooked)
{
    //
    // Initialize the members
    //
    ::RtlInitUnicodeString(&Path, UnicodePath->Buffer);

    dbg("Creating HookedDriver('%S')", UnicodePath->Buffer);
    dbg("Creating HookedDriver('%S')", Path.Buffer);

    //
    // Increment the refcount to the driver
    //
    NTSTATUS Status = ::ObReferenceObjectByName(
        &Path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        nullptr,
        (PVOID*)&DriverObject);

    NT_ASSERT(NT_SUCCESS(Status));
    NT_ASSERT(DriverObject != nullptr);

    //
    // Swap the callbacks of the driver
    //
    // SwapCallbacks();
}

HookedDriver::~HookedDriver()
{
    dbg("Destroying HookedDriver '%S'", Path.Buffer);

    Enabled = false;

    //
    // Restore the callbacks
    //
    // RestoreCallbacks();

    //
    // Decrements the refcount to the DriverObject
    //
    ObDereferenceObject(DriverObject);
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
            (PVOID*)&DriverObject->MajorFunction[i],
            (PVOID)Callbacks::InterceptGenericRoutine);

        OriginalRoutines[i] = OldRoutine;
    }

    //
    // Hook FastIo too
    //
    if ( DriverObject->FastIoDispatch )
    {
        PFAST_IO_DEVICE_CONTROL OldFastIoDeviceControl = (PFAST_IO_DEVICE_CONTROL)InterlockedExchangePointer(
            (PVOID*)&DriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID)Callbacks::InterceptGenericFastIoDeviceControl);

        FastIoDeviceControl = OldFastIoDeviceControl;

        PFAST_IO_READ OldFastIoRead = (PFAST_IO_READ)InterlockedExchangePointer(
            (PVOID*)&DriverObject->FastIoDispatch->FastIoRead,
            (PVOID)Callbacks::InterceptGenericFastIoRead);

        FastIoRead = OldFastIoRead;

        PFAST_IO_WRITE OldFastIoWrite = (PFAST_IO_WRITE)InterlockedExchangePointer(
            (PVOID*)&DriverObject->FastIoDispatch->FastIoWrite,
            (PVOID)Callbacks::InterceptGenericFastIoWrite);

        FastIoWrite = OldFastIoWrite;
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
    // Restore the original callbacks
    //

    for ( u16 i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ )
    {
        InterlockedExchangePointer((PVOID*)&DriverObject->MajorFunction[i], (PVOID)OriginalRoutines[i]);
    }

    //
    // Restore Fast IO Dispatch pointers
    //

    if ( DriverObject->FastIoDispatch )
    {
        InterlockedExchangePointer(
            (PVOID*)DriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID)FastIoDeviceControl);

        InterlockedExchangePointer((PVOID*)&DriverObject->FastIoDispatch->FastIoRead, (PVOID)FastIoRead);

        InterlockedExchangePointer((PVOID*)&DriverObject->FastIoDispatch->FastIoWrite, (PVOID)FastIoWrite);
    }

    //
    // Switch back state flag
    //
    State = HookState::Hooked;
}
} // namespace CFB::Driver
