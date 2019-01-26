#pragma once

#include "Common.h"
#include "Driver.h"

#define CFB_TESTCASE_TAG 0x54424643
#define CFB_MAX_TESTCASE_SIZE 0x1000

NTSTATUS InitializeTestCaseStructures();
NTSTATUS HandleIoStoreTestCase(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS ReleaseTestCaseStructures();
