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

    dbg("HookedDriverManager::InsertDriver(): Found driver at %p", pDriver);

    //
    // Refuse to hook IrpMonitor
    //
    if ( pDriver == Globals->DriverObject )
    {
        ObDereferenceObject(pDriver);
        return STATUS_ACCESS_DENIED;
    }

    {
        Utils::ScopedLock lock(SpinLock);

        //
        // Check if the driver is already hooked
        //
        auto IsDriverHooked = [&pDriver](const HookedDriver* h)
        {
            return h->DriverObject == pDriver;
        };

        if ( Entries.Find(IsDriverHooked) != nullptr )
        {
            ObDereferenceObject(pDriver);
            return STATUS_ALREADY_REGISTERED;
        }

        dbg("HookedDriverManager::InsertDriver(): Driver %p is not hooked, hooking now...", pDriver);

        //
        // Allocate the new HookedDriver, this will result in the IRP_MJ_* being redirected
        //
        auto New = new HookedDriver(Path, pDriver);

        //
        // Last, insert the driver to the linked list
        //
        Entries += New;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HookedDriverManager::RemoveDriver(const wchar_t* Path)
{
    Utils::ScopedLock lock(this->SpinLock);

    UNICODE_STRING UnicodePath = {0};
    ::RtlInitUnicodeString(&UnicodePath, Path);

    const usize PathMaxLength = min(UnicodePath.Length, CFB_DRIVER_MAX_PATH);

    auto FromDriverPath = [&UnicodePath, &PathMaxLength](const HookedDriver* h)
    {
        return ::RtlCompareUnicodeString(&h->Path, &UnicodePath, true) == 0;
    };

    auto MatchedDriver = Entries.Find(FromDriverPath);
    if ( MatchedDriver == nullptr )
    {
        return STATUS_NOT_FOUND;
    }

    Entries -= MatchedDriver;
    return STATUS_SUCCESS;
}

} // namespace CFB::Driver
