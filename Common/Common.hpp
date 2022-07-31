#ifdef CFB_KERNEL_DRIVER
// clang-format off
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
// clang-format on
#else
#include <windows.h>
#endif // CFB_KERNEL_DRIVER

//
// Types
//

///
/// Static types
///
using u8  = UINT8;
using u16 = UINT16;
using u32 = UINT32;
using u64 = UINT64;

using i8  = INT8;
using i16 = INT16;
using i32 = INT32;
using i64 = INT64;

using usize = SIZE_T;
using uptr  = ULONG_PTR;

// clang-format off
#define CFB_DEVICE_NAME                   L"IrpMonitor"
#define CFB_DEVICE_PATH                   L"\\Device\\" CFB_DEVICE_NAME
#define CFB_DOS_DEVICE_PATH               L"\\??\\" CFB_DEVICE_NAME
#define CFB_USER_DEVICE_PATH              L"\\\\.\\" CFB_DEVICE_NAME
#define CFB_DEVICE_TAG                    ' BFC'

#define CFB_DRIVER_MAX_PATH               256
#define CFB_DRIVER_BASENAME               CFB_DEVICE_NAME L".sys"

#define CFB_BROKER_SERVICE_NAME           L"CFB_BROKER"
#define CFB_BROKER_SERVICE_DESCRIPTION    L"Canadian Furious Beaver"

#ifndef countof
#define countof(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#endif

#ifndef boolstr
#define boolstr(x)                  ( (x) ? "true" : "false")
#endif

#ifndef boolstrw
#define boolstrw(x)                 ( (x) ? L"true" : L"false")
#endif
// clang-format on
