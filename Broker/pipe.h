#pragma once

#include <Windows.h>

#include "common.h"
#include "main.h"

static HANDLE g_hServerPipe = INVALID_HANDLE_VALUE;

BOOL CreateServerPipe();
BOOL CloseServerPipe();
_Success_(return) BOOL StartGuiThread(_Out_ PHANDLE lpThread);