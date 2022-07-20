#include "HookedDriverManager.hpp"

#include "Context.hpp"
#include "Native.hpp"


namespace CFB::Driver
{
HookedDriverManager::HookedDriverManager()
{
    dbg("Creating HookedDriverManager");
}

HookedDriverManager::~HookedDriverManager()
{
    dbg("Destroying HookedDriverManager");
}

NTSTATUS
HookedDriverManager::InsertDriver(const wchar_t* Path)
{
    PDRIVER_OBJECT pDriver     = nullptr;
    UNICODE_STRING UnicodePath = {0};

    ::RtlInitUnicodeString(&UnicodePath, Path);

    dbg("HookedDriverManager::InsertDriver('%S', %d)", UnicodePath.Buffer, UnicodePath.Length);

    //
    // Resolve the given `Path` parameter as name for Driver Object
    //
    {
        NTSTATUS Status = ::ObReferenceObjectByName(
            &UnicodePath,
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
        Utils::ScopedWrapper DriverObj(
            pDriver,
            [&pDriver]()
            {
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
            auto FromDriverAddress = [&DriverObj](const HookedDriver* h)
            {
                return h->DriverObject == DriverObj.get();
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

            dbg("HookedDriverManager::InsertDriver(): Driver %p is not hooked, hooking now...", DriverObj.get());

            //
            // Allocate the new HookedDriver, this will result in all the `IRP_MJ_*` of the driver being
            // redirected to IrpMonitor
            //
            auto NewHookedDriver = new HookedDriver(Path);

            //
            // Last, insert the driver to the linked list
            //
            Entries += NewHookedDriver;

            dbg("Added '%S' to the hooked driver list", NewHookedDriver->Path.Buffer);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveDriver(const wchar_t* Path)
{
    UNICODE_STRING UnicodePath = {0};
    ::RtlInitUnicodeString(&UnicodePath, Path);

    const usize PathMaxLength = min(UnicodePath.Length, CFB_DRIVER_MAX_PATH);
    dbg("Trying to remove '%S' from the hooked driver list", UnicodePath.Buffer);

    {
        Utils::ScopedLock lock(Mutex);
        auto FromDriverPath = [&UnicodePath, &PathMaxLength](const HookedDriver* h)
        {
            return ::RtlCompareUnicodeString(&h->Path, &UnicodePath, true) == 0;
        };

        auto MatchedDriver = Entries.Find(FromDriverPath);
        if ( MatchedDriver == nullptr )
        {
            return STATUS_NOT_FOUND;
        }

        dbg("Removing HookedDriver %p ...", MatchedDriver);

        Entries -= MatchedDriver;
        delete MatchedDriver;
    }

    return STATUS_SUCCESS;
}

} // namespace CFB::Driver
