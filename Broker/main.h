#pragma once
#include <Windows.h>
#include <malloc.h>

#include "common.h"
#include "pipe.h"
#include "driver.h"

#pragma comment(lib, "advapi32.lib") // for privilege check and driver/service (un-)loading

// TODO: make a Session class instead of globals
BOOL g_bIsRunning = FALSE;
TaskManager g_RequestManager;
TaskManager g_ResponseManager;