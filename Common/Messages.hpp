#pragma once

#include "Common.hpp"

union IoMessage
{
    wchar_t DriverName[CFB_DRIVER_MAX_PATH];
    HANDLE IrpNotificationEventHandle;
};
