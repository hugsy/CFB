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
    State(HookState::Unhooked),
    Path(UnicodePath->Buffer, NonPagedPoolNx)
{
    dbg("Creating HookedDriver('%wZ')", &Path);

    //
    // Initialize the members
    //

    //
    // Increment the refcount to the driver
    //
    NTSTATUS Status = ::ObReferenceObjectByName(
        Path.get(),
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
    SwapCallbacks();
}

HookedDriver::~HookedDriver()
{
    dbg("Destroying HookedDriver '%wZ'", Path.get());

    DisableCapturing();

    //
    // Restore the callbacks
    //
    RestoreCallbacks();

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
    State = HookState::Unhooked;
}

bool
HookedDriver::EnableCapturing()
{
    Utils::ScopedLock lock(Mutex);
    bool bStatusChanged = false;

    if ( State == HookState::Hooked )
    {
        Enabled        = true;
        bStatusChanged = true;
    }
    dbg("State %schanged, current value %s", (bStatusChanged ? "" : "un"), boolstr(HasCapturingEnabled()));
    return bStatusChanged;
}

bool
HookedDriver::DisableCapturing()
{
    Utils::ScopedLock lock(Mutex);
    bool bStatusChanged = false;
    Enabled             = false;
    bStatusChanged      = true;
    dbg("State %schanged, current value %s", (bStatusChanged ? "" : "un"), boolstr(HasCapturingEnabled()));
    return bStatusChanged;
}


bool
HookedDriver::HasCapturingEnabled() const
{
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
