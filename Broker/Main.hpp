#pragma once

#include "common.h"
#include "Session.h"
#include "FrontEndServer.h"
#include "driver.h"

#pragma comment(lib, "advapi32.lib") // for privilege check and driver/service (un-)loading

#include <iostream>


DWORD RunForever(LPVOID lpParameter);

