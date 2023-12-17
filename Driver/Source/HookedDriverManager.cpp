#include "HookedDriverManager.hpp"

#include "Context.hpp"
#include "Native.hpp"

#define xerr(fmt, ...)                                                                                                 \
    {                                                                                                                  \
        err("[HookedDriverManager] " fmt, __VA_ARGS__);                                                                \
    }

#define xinfo(fmt, ...)                                                                                                \
    {                                                                                                                  \
        info("[HookedDriverManager] " fmt, __VA_ARGS__);                                                               \
    }

#define xdbg(fmt, ...)                                                                                                 \
    {                                                                                                                  \
        dbg("[HookedDriverManager] " fmt, __VA_ARGS__);                                                                \
    }

namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
HookedDriverManager::HookedDriverManager()
{
    xdbg("Creating HookedDriverManager");
}

HookedDriverManager::~HookedDriverManager()
{
    xdbg("Destroying HookedDriverManager");

    HookedDriverManager::RemoveAllDrivers();
}

NTSTATUS
HookedDriverManager::InsertDriver(Utils::KUnicodeString const& UnicodePath)
{
    NTSTATUS Status        = STATUS_UNSUCCESSFUL;
    PDRIVER_OBJECT pDriver = nullptr;

    xdbg("HookedDriverManager::InsertDriver('%wZ')", UnicodePath.get());

    //
    // Resolve the given `Path` parameter as name for Driver Object
    //
    {
        Status = ::ObReferenceObjectByName(
            UnicodePath.get(),
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            nullptr,
            0,
            *IoDriverObjectType,
            KernelMode,
            nullptr,
            (PVOID*)&pDriver);

        if ( !NT_SUCCESS(Status) )
        {
            return Status;
        }
    }


    xdbg("HookedDriverManager::InsertDriver(): Found driver at %p", pDriver);

    //
    // On any failure, make sure to dereference the object
    //
    Utils::ScopedWrapper ScopedDriverObject(
        pDriver,
        [&pDriver]()
        {
            //
            // Always dereference of scope-leave. `HookedDriver` manages its own driver reference
            //
            xdbg("HookedDriverManager::InsertDriver(): dereferencing object %p", pDriver);
            ObDereferenceObject(pDriver);
        });

    //
    // Refuse to hook IrpMonitor
    //
    if ( pDriver == Globals->DriverObject )
    {
        xerr("HookedDriverManager::InsertDriver(): refusing to hook %S", CFB_DRIVER_BASENAME);
        return STATUS_ACCESS_DENIED;
    }


    Utils::ScopedLock lock(m_Mutex);

    //
    // Check if the driver is already hooked
    //
    auto FromDriverAddress = [&ScopedDriverObject](const HookedDriver* h)
    {
        return h->OriginalDriverObject == ScopedDriverObject.get();
    };

    if ( m_Entries.Find(FromDriverAddress) != nullptr )
    {
        return STATUS_ALREADY_REGISTERED;
    }

    //
    // Check if there's space
    //
    if ( m_Entries.Size() >= CFB_MAX_HOOKED_DRIVERS )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    xdbg(
        "HookedDriverManager::InsertDriver(): Driver '%wZ' (%p) is not hooked, hooking now...",
        UnicodePath.get(),
        ScopedDriverObject.get());

    //
    // Allocate the new HookedDriver, this will result in all the `IRP_MJ_*` of the driver being
    // redirected to IrpMonitor
    //
    auto NewHookedDriver = new HookedDriver(UnicodePath);

    //
    // Last, insert the driver to the linked list
    //
    m_Entries.PushBack(NewHookedDriver);

    xdbg("Added '%wZ' to the hooked driver list, TotalEntries=%d", NewHookedDriver->Path.get(), m_Entries.Size());

    xinfo("Driver '%wZ' is hooked", NewHookedDriver->Path.get());

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveDriver(Utils::KUnicodeString const& UnicodePath)
{
    Utils::ScopedLock lock(m_Mutex);

    const usize PathMaxLength = min(UnicodePath.size(), CFB_DRIVER_MAX_PATH);
    auto FromDriverPath       = [&UnicodePath, &PathMaxLength](HookedDriver* h)
    {
        return h->Path == UnicodePath;
    };

    auto MatchedDriver = m_Entries.Find(FromDriverPath);
    if ( MatchedDriver == nullptr )
    {
        return STATUS_NOT_FOUND;
    }

    xdbg("Removing HookedDriver '%wZ' (%p) ...", MatchedDriver->Path.get(), MatchedDriver);

    m_Entries -= MatchedDriver;

    ObDereferenceObject(MatchedDriver->OriginalDriverObject);

    xinfo("Driver '%wZ' is unhooked", MatchedDriver->Path.get());
    delete MatchedDriver;

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveAllDrivers()
{
    xdbg("Removing all drivers");

    Utils::ScopedLock lock(m_Mutex);
    do
    {
        auto Entry = m_Entries.PopBack();
        if ( Entry == nullptr )
        {
            break;
        }
        delete Entry;
    } while ( true );

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::SetMonitoringState(const PUNICODE_STRING UnicodePath, bool bEnable)
{
    Utils::ScopedLock lock(m_Mutex);

    const usize PathMaxLength = min(UnicodePath->Length, CFB_DRIVER_MAX_PATH);
    auto FromDriverPath       = [&UnicodePath, &PathMaxLength](HookedDriver* h)
    {
        return h->Path == UnicodePath;
    };

    auto MatchedDriver = m_Entries.Find(FromDriverPath);
    if ( MatchedDriver == nullptr )
    {
        return STATUS_NOT_FOUND;
    }

    const bool OldState = MatchedDriver->CanCapture();
    (bEnable) ? MatchedDriver->EnableCapturing() : MatchedDriver->DisableCapturing();
    const bool DriverStateChanged = OldState != MatchedDriver->CanCapture();

    xdbg(
        "HookedDriverManager::SetMonitoringState('%wZ', %s): state %schanged, CanCapture=%s",
        MatchedDriver->Path.get(),
        boolstr(bEnable),
        (DriverStateChanged ? "" : "un"),
        boolstr(MatchedDriver->CanCapture()));

    xinfo(
        "Capture of IRPs to driver '%wZ' are %scaptured",
        MatchedDriver->Path.get(),
        MatchedDriver->CanCapture() ? "" : "not ");

    return STATUS_SUCCESS;
}

Utils::LinkedList<HookedDriver>&
HookedDriverManager::Items()
{
    return m_Entries;
}
} // namespace CFB::Driver
