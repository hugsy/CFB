#include "HookedDriver.hpp"


namespace Callbacks = CFB::Driver::Callbacks;
namespace Utils     = CFB::Driver::Utils;

namespace CFB::Driver
{
HookedDriver::HookedDriver(const wchar_t* _Path, const PDRIVER_OBJECT _DriverObject) :
    Enabled(false),
    DriverObject(_DriverObject),
    Next(),
    OriginalRoutines(),
    FastIoRead(nullptr),
    FastIoWrite(nullptr),
    FastIoDeviceControl(nullptr),
    InterceptedIrpsCount(0)
{
    dbg("Creating HookedDriver('%S')", _Path);

    //
    // Initialize the members
    //
    ::RtlInitUnicodeString(&Path, _Path);

    //
    // Swap the callbacks of the driver
    //
    // SwapCallbacks();
}

HookedDriver::~HookedDriver()
{
    dbg("Destroying HookedDriver '%wS'", Path);

    //
    // Restore the callbacks
    //
    /// TODO:

    //
    // Decrements the refcount to the DriverObject
    //
    ObDereferenceObject(DriverObject);
}

void
HookedDriver::SwapCallbacks()
{
    Utils::ScopedLock lock(Mutex);

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
}
} // namespace CFB::Driver
