#pragma once

#include "Common.h"
#include "Driver.h"

#define CFB_TESTCASE_TAG 'CFBT'
#define CFB_MAX_TESTCASE_SIZE 4096

NTSTATUS InitializeTestCaseStructures();
NTSTATUS HandleIoStoreTestCase(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS ReleaseTestCaseStructures();
