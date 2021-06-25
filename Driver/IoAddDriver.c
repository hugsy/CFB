#include "IoAddDriver.h"

typedef enum
{
    Driver,
    FileSystem,
    Device
} HOOKABLE_OBJECT_T;

extern PLIST_ENTRY g_HookedDriverHead;




VOID InitializeIoAddDriverStructure()
{
    KeInitializeSpinLock(&g_AddRemoveDriverSpinLock);
}



/*++

--*/
NTSTATUS AddObjectByName(LPWSTR lpObjectName, HOOKABLE_OBJECT_T Type)
{
    NTSTATUS Status = STATUS_SUCCESS;

    /* make sure the list is not full */
    if (GetNumberOfHookedDrivers() == CFB_MAX_HOOKED_DRIVERS)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* lookup the driver by name, and swap it if found */
    UNICODE_STRING UnicodeName;
    PDRIVER_OBJECT pDriver;
    PDEVICE_OBJECT pDevice;
    


    RtlInitUnicodeString(&UnicodeName, lpObjectName);

    switch (Type)
    {
    case Driver:
    case FileSystem:

        Status = ObReferenceObjectByName(
            /* IN PUNICODE_STRING */ &UnicodeName,
            /* IN ULONG */ OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            /* IN PACCESS_STATE */ NULL,
            /* IN ACCESS_MASK */ 0,
            /* IN POBJECT_TYPE */ *IoDriverObjectType,
            /* IN KPROCESSOR_MODE */KernelMode,
            /* IN OUT PVOID */ NULL,
            /* OUT PVOID* */ (PVOID*)&pDriver
        );

        if (!NT_SUCCESS(Status))
            return Status;


        break;

    case Device:
        
        Status = ObReferenceObjectByName(
            &UnicodeName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            0,
            *IoDeviceObjectType,
            KernelMode,
            NULL,
            (PVOID*)&pDevice
        );

        // always returns 0xc0000024 
        

        if (!NT_SUCCESS(Status))
            return Status;


        pDriver = pDevice->DriverObject;
        
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }


    //
    // check if driver is already hooked
    //

    if ( IsDriverHooked(pDriver) )
    {
        return STATUS_ALREADY_REGISTERED;
    }


    //
    // create the new hooked driver pool object, and chain it to the rest
    //

    PHOOKED_DRIVER NewDriver = ExAllocatePoolWithTag(NonPagedPool, sizeof(HOOKED_DRIVER), CFB_DEVICE_TAG);
    if (!NewDriver)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlSecureZeroMemory(NewDriver, sizeof(HOOKED_DRIVER));


    wcscpy_s(NewDriver->Name, sizeof(NewDriver->Name) / sizeof(wchar_t), lpObjectName);
    RtlUnicodeStringCopy(&NewDriver->UnicodeName, &UnicodeName);
    NewDriver->DriverObject = pDriver;

    

    KeAcquireInStackQueuedSpinLock(&g_AddRemoveDriverSpinLock, &g_AddRemoveSpinLockQueue);

    //
    // Exchange pointers for all the major functions
    //
    for (DWORD i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        PDRIVER_DISPATCH OldRoutine = (PDRIVER_DISPATCH)InterlockedExchangePointer(
            (PVOID*)&pDriver->MajorFunction[i],
            (PVOID)InterceptGenericRoutine
        );

        NewDriver->OriginalRoutines[i] = OldRoutine;
    }


    //
    // Exchange pointer for Fast IO dispatcher
    //
    
    if (pDriver->FastIoDispatch)
    {
        PFAST_IO_DEVICE_CONTROL OldFastIoDeviceControl = (PFAST_IO_DEVICE_CONTROL)InterlockedExchangePointer(
            (PVOID*)&pDriver->FastIoDispatch->FastIoDeviceControl,
            (PVOID)InterceptGenericFastIoDeviceControl
        );

        NewDriver->FastIoDeviceControl = OldFastIoDeviceControl;

        PFAST_IO_READ OldFastIoRead = (PFAST_IO_READ)InterlockedExchangePointer(
            (PVOID*)&pDriver->FastIoDispatch->FastIoRead,
            (PVOID)InterceptGenericFastIoRead
        );

        NewDriver->FastIoRead = OldFastIoRead;

        PFAST_IO_WRITE OldFastIoWrite = (PFAST_IO_WRITE)InterlockedExchangePointer(
            (PVOID*)&pDriver->FastIoDispatch->FastIoWrite,
            (PVOID)InterceptGenericFastIoWrite
        );

        NewDriver->FastIoWrite = OldFastIoWrite;
    }

    //
    // add it to the list
    //
    InsertTailList(g_HookedDriverHead, &(NewDriver->ListEntry));

    //
    // Set the driver as ready
    //
    NewDriver->Enabled = TRUE;

    KeReleaseInStackQueuedSpinLock(&g_AddRemoveSpinLockQueue);

    return Status;
}


/*++

--*/
NTSTATUS HandleIoAddDriver(PIRP Irp, PIO_STACK_LOCATION Stack)
{

    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR lpObjectName;
    ULONG InputBufferLen;

    do
    {

        //
        // Check IRP arguments
        //

        lpObjectName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;

        if (!lpObjectName)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        InputBufferLen = Stack->Parameters.DeviceIoControl.InputBufferLength;

        if (InputBufferLen >= HOOKED_DRIVER_MAX_NAME_LEN)
        {
            CfbDbgPrintErr(L"Input buffer too large\n");
            Status = STATUS_BUFFER_OVERFLOW;
            break;
        }


        CfbDbgPrintInfo(L"Adding AddObjectByName('%s') \n", lpObjectName);


        //
        // Add the driver
        //

        //
        // Cannot hook itself
        //
        if (wcsncmp(lpObjectName, CFB_DRIVER_PATH, wcslen(CFB_DRIVER_PATH)) == 0)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (wcsncmp(lpObjectName, L"\\driver", 7) == 0)
        {
            Status = AddObjectByName(lpObjectName, Driver);
        }
        else if (wcsncmp(lpObjectName, L"\\filesystem", 11) == 0)
        {
            Status = AddObjectByName(lpObjectName, FileSystem);
        }
        else if (wcsncmp(lpObjectName, L"\\device", 7) == 0)
        {
            Status = AddObjectByName(lpObjectName, Device);
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        
        CfbDbgPrintOk(L"AddObjectByName('%s') returned %#x\n", lpObjectName, Status);
    }
    while(0);

    return Status;
}
