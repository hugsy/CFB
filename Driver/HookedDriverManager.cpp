#include "HookedDriverManager.hpp"

#include "Context.hpp"
#include "Native.hpp"


namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{
HookedDriverManager::HookedDriverManager()
{
    dbg("Creating HookedDriverManager");
}

HookedDriverManager::~HookedDriverManager()
{
    dbg("Destroying HookedDriverManager");

    HookedDriverManager::RemoveAllDrivers();
}

NTSTATUS
HookedDriverManager::InsertDriver(const wchar_t* Path)
{
    return InsertDriver(Utils::KUnicodeString(Path).get());
}

NTSTATUS
HookedDriverManager::InsertDriver(const PUNICODE_STRING UnicodePath)
{
    PDRIVER_OBJECT pDriver = nullptr;

    dbg("HookedDriverManager::InsertDriver('%wZ', %p, %d)", UnicodePath, UnicodePath, UnicodePath->Length);

    //
    // Resolve the given `Path` parameter as name for Driver Object
    //
    {
        NTSTATUS Status = ::ObReferenceObjectByName(
            UnicodePath,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
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

    {
        dbg("HookedDriverManager::InsertDriver(): Found driver at %p", pDriver);
        Utils::ScopedWrapper ScopedDriverObject(
            pDriver,
            [&pDriver]()
            {
                dbg("HookedDriverManager::InsertDriver(): derefencing object %p", pDriver);
                ObDereferenceObject(pDriver);
            });

        //
        // Refuse to hook IrpMonitor
        //
        if ( pDriver == Globals->DriverObject )
        {
            return STATUS_ACCESS_DENIED;
        }

        {
            Utils::ScopedLock lock(Mutex);

            //
            // Check if the driver is already hooked
            //
            auto FromDriverAddress = [&ScopedDriverObject](const HookedDriver* h)
            {
                return h->DriverObject == ScopedDriverObject.get();
            };

            if ( Entries.Find(FromDriverAddress) != nullptr )
            {
                return STATUS_ALREADY_REGISTERED;
            }

            //
            // Check if there's space
            //
            if ( Entries.Size() >= CFB_MAX_HOOKED_DRIVERS )
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            dbg("HookedDriverManager::InsertDriver(): Driver '%wZ' (%p) is not hooked, hooking now...",
                UnicodePath,
                ScopedDriverObject.get());

            //
            // Allocate the new HookedDriver, this will result in all the `IRP_MJ_*` of the driver being
            // redirected to IrpMonitor
            //
            auto NewHookedDriver = new HookedDriver(UnicodePath);

            //
            // Last, insert the driver to the linked list
            //
            Entries += NewHookedDriver;

            dbg("Added '%wZ' to the hooked driver list, TotalEntries=%d", NewHookedDriver->Path.get(), Entries.Size());
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveDriver(const wchar_t* Path)
{
    return RemoveDriver(Utils::KUnicodeString(Path).get());
}

NTSTATUS
HookedDriverManager::RemoveDriver(const PUNICODE_STRING UnicodePath)
{
    Utils::ScopedLock lock(Mutex);

    const usize PathMaxLength = min(UnicodePath->Length, CFB_DRIVER_MAX_PATH);
    auto FromDriverPath       = [&UnicodePath, &PathMaxLength](HookedDriver* h)
    {
        return h->Path == UnicodePath;
    };

    auto MatchedDriver = Entries.Find(FromDriverPath);
    if ( MatchedDriver == nullptr )
    {
        return STATUS_NOT_FOUND;
    }

    dbg("Removing HookedDriver '%wZ' (%p) ...", MatchedDriver->Path.get(), MatchedDriver);

    Entries -= MatchedDriver;
    delete MatchedDriver;

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveAllDrivers()
{
    dbg("Removing all drivers");

    Utils::ScopedLock lock(Mutex);

    do
    {
        auto Entry = Entries.PopTail();
        if ( Entry == nullptr )
            break;
        delete Entry;
    } while ( true );

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::SetMonitoringState(const wchar_t* Path, bool bEnable)
{
    UNICODE_STRING UnicodePath = RTL_CONSTANT_STRING(Path);
    return SetMonitoringState(&UnicodePath, bEnable);
}

NTSTATUS
HookedDriverManager::SetMonitoringState(const PUNICODE_STRING UnicodePath, bool bEnable)
{
    Utils::ScopedLock lock(Mutex);

    const usize PathMaxLength = min(UnicodePath->Length, CFB_DRIVER_MAX_PATH);
    auto FromDriverPath       = [&UnicodePath, &PathMaxLength](HookedDriver* h)
    {
        return h->Path == UnicodePath;
    };

    auto MatchedDriver = Entries.Find(FromDriverPath);
    if ( MatchedDriver == nullptr )
    {
        return STATUS_NOT_FOUND;
    }

    dbg("HookedDriverManager::SetMonitoringState('%wZ', %s)", MatchedDriver->Path.get(), boolstr(bEnable));

    const bool DriverStateChanged = (bEnable) ? MatchedDriver->EnableCapturing() : MatchedDriver->DisableCapturing();

    return DriverStateChanged ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

} // namespace CFB::Driver
