#pragma once

//
// We want to use the NTSTATUS codes from the broker, quiet down
// the compiler warnings
//
#pragma warning (push)
#pragma warning (disable: 4005)
#include <ntstatus.h>
#define WIN32_NO_STATUS
#define UMDF_USING_NTSTATUS
#pragma warning (pop)

#include <Windows.h>

#include <stdio.h>
#include <wchar.h>

extern "C"
{
#include "..\Common\Common.h"
#include "..\Driver\IoctlCodes.h"
}

#include "CfbException.h"