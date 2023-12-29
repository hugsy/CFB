#define CFB_NS "[CFB::Driver::HookedDriver]"

#include "HookedDriver.hpp"

#include "Native.hpp"

namespace Callbacks = CFB::Driver::Callbacks;


namespace CFB::Driver
{
HookedDriver::HookedDriver(Utils::KUnicodeString const& UnicodePath) :
    Next {},
    OriginalDriverObject {nullptr},
    HookedDriverObject {new(NonPagedPoolNx) DRIVER_OBJECT},
    Path {UnicodePath},
    m_CapturingEnabled {false},
    m_State {HookState::Unhooked},
    m_InterceptedIrpsCount {0}
{
    dbg("Creating HookedDriver('%wZ')", Path.get());

    //
    // Find the driver from its name, link reference to the DRIVER_OBJECT to the lifetime of this HookedDriver
    //
    NTSTATUS Status = ::ObReferenceObjectByName(
        Path.get(),
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr,
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
    ::memcpy(HookedDriverObject.get(), OriginalDriverObject, sizeof(DRIVER_OBJECT));

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
    Utils::ScopedLock lock(m_CallbackLock);

    if ( m_State != HookState::Unhooked )
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
            (PVOID*)&HookedDriverObject->MajorFunction[i],
            (PVOID)Callbacks::InterceptGenericRoutine);
    }

    //
    // Hook FastIo too
    //
    if ( OriginalDriverObject->FastIoDispatch )
    {
        PFAST_IO_DEVICE_CONTROL OldFastIoDeviceControl = (PFAST_IO_DEVICE_CONTROL)InterlockedExchangePointer(
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoDeviceControl,
            (PVOID)Callbacks::InterceptGenericFastIoDeviceControl);

        PFAST_IO_READ OldFastIoRead = (PFAST_IO_READ)InterlockedExchangePointer(
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoRead,
            (PVOID)Callbacks::InterceptGenericFastIoRead);

        PFAST_IO_WRITE OldFastIoWrite = (PFAST_IO_WRITE)InterlockedExchangePointer(
            (PVOID*)&HookedDriverObject->FastIoDispatch->FastIoWrite,
            (PVOID)Callbacks::InterceptGenericFastIoWrite);
    }

    //
    // This is where the hooking takes places: for each device object of the original driver, we replace
    // the driver object pointer member making it point to our hooked version.
    //
    for ( PDEVICE_OBJECT DeviceObject = OriginalDriverObject->DeviceObject; DeviceObject != nullptr;
          DeviceObject                = DeviceObject->NextDevice )
    {
        InterlockedExchangePointer((PVOID*)&DeviceObject->DriverObject, (PVOID)HookedDriverObject.get());
    }

    //
    // Switch state flag to hooked
    //
    m_State = HookState::Hooked;
}

void
HookedDriver::RestoreCallbacks()
{
    Utils::ScopedLock lock(m_CallbackLock);

    if ( m_State != HookState::Hooked )
    {
        warn("Invalid state: expecting 'Hooked'");
        return;
    }

    //
    // Switch back state flag
    //
    m_State = HookState::Unhooked;

    //
    // For each device object, restore the original driver object pointer
    //
    for ( PDEVICE_OBJECT DeviceObject = OriginalDriverObject->DeviceObject; DeviceObject != nullptr;
          DeviceObject                = DeviceObject->NextDevice )
    {
        InterlockedExchangePointer((PVOID*)&DeviceObject->DriverObject, (PVOID)OriginalDriverObject);
    }
}


bool
HookedDriver::EnableCapturing()
{
    Utils::ScopedLock lock(m_CallbackLock);
    m_CapturingEnabled = true;
    return true;
}


bool
HookedDriver::DisableCapturing()
{
    Utils::ScopedLock lock(m_CallbackLock);
    m_CapturingEnabled = false;
    return true;
}


bool
HookedDriver::CanCapture()
{
    dbg("Driver('%wZ', State=%shooked, Enabled=%s)",
        Path.get(),
        (m_State == HookState::Unhooked) ? "un" : "",
        boolstr(m_CapturingEnabled));
    return m_State == HookState::Hooked && m_CapturingEnabled == true;
}


usize const
HookedDriver::IrpCount() const
{
    return m_InterceptedIrpsCount;
}


void
HookedDriver::IncrementIrpCount()
{
    InterlockedIncrement64((PLONG64)&m_InterceptedIrpsCount);
}


void
HookedDriver::DecrementIrpCount()
{
    InterlockedDecrement64((PLONG64)&m_InterceptedIrpsCount);
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
