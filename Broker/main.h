#pragma once
#include <Windows.h>
#include <malloc.h>


#include "common.h"
#include "pipe.h"
#include "driver.h"

#pragma comment(lib, "Advapi32.lib") // for privilege check and driver/service {un}loading

BOOL g_bIsRunning;