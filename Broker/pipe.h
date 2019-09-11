#pragma once

#include <Windows.h>
#include <cstddef>

#include "common.h"
#include "task.h"
#include "taskmanager.h"
#include "queue.h"

static HANDLE g_hServerPipe = INVALID_HANDLE_VALUE;

extern BOOL g_bIsRunning;
extern TaskManager g_RequestManager;
extern TaskManager g_ResponseManager;

BOOL CreateServerPipe();
BOOL CloseServerPipe();
_Success_(return) BOOL StartFrontendManagerThread(_In_ PHANDLE pEvent, _Out_ PHANDLE lpThread);