#pragma once

#include <Windows.h>


HANDLE OpenDevice();
BOOLEAN CloseDevice(HANDLE);
BOOLEAN QueryDevice(HANDLE);
