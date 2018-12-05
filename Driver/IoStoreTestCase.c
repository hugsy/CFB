#include "IoStoreTestCase.h"

PVOID g_LastTestCase;

static FAST_MUTEX FastMutex;


/*++
--*/
VOID InitializeTestCaseStructures()
{
    ExInitializeFastMutex(&FastMutex);

    ExAcquireFastMutex(&FastMutex);
    g_LastTestCase = ExAllocatePoolWithTag(PagedPool, CFB_MAX_TESTCASE_SIZE, CFB_TESTCASE_TAG);
    ExReleaseFastMutex(&FastMutex);
}


/*++

--*/
NTSTATUS HandleIoStoreTestCase(PIRP Irp, PIO_STACK_LOCATION Stack) 
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;


    if (g_LastTestCase)
    {
        return STATUS_ALREADY_REGISTERED;
    }


    ExAcquireFastMutex(&FastMutex);

    RtlSecureZeroMemory(g_LastTestCase, CFB_MAX_TESTCASE_SIZE);

    __try {
        do
        {

            PVOID lpUserBuffer = Irp->AssociatedIrp.SystemBuffer;
            UINT32 uBufferLen = (UINT32) Stack->Parameters.DeviceIoControl.InputBufferLength;

            if (uBufferLen-sizeof(PUINT32) > CFB_MAX_TESTCASE_SIZE)
            {
                CfbDbgPrintErr(L"Cannot store testcase (size=0x%x, max=0x%x)\n", uBufferLen, CFB_MAX_TESTCASE_SIZE);
                Status = STATUS_BUFFER_OVERFLOW;
                break;
            }

            //ProbeForRead(lpUserBuffer, uBufferLen, (ULONG)__alignof(uBufferLen));

            PUINT32 ptr = (PUINT32)g_LastTestCase;
            *ptr = uBufferLen;
            RtlCopyMemory((PVOID)((PUINT32)g_LastTestCase + 1), lpUserBuffer, uBufferLen - sizeof(PUINT32));

            Status = STATUS_SUCCESS;
        } 
        while (0);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        Status = GetExceptionCode();
        CfbDbgPrintErr(L"Exception Code: 0x%x\n", Status);
    }

    ExReleaseFastMutex(&FastMutex);

    return Status;

}


/*++

--*/
NTSTATUS ReleaseTestCaseStructures()
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    ExAcquireFastMutex(&FastMutex);
    
    if (g_LastTestCase)
    {
        ExFreePoolWithTag(g_LastTestCase, CFB_TESTCASE_TAG);
        g_LastTestCase = NULL;
        Status = STATUS_SUCCESS;
    }

    ExReleaseFastMutex(&FastMutex);

    return Status;
}