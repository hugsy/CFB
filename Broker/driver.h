#pragma once

#include <memory>

#include "common.h"
#include "Session.h"
#include "SafeHandle.h"


_Success_(return) BOOL StartBackendManagerThread(_In_ PVOID lpParameter);
