// clang-format off
#ifdef CFB_KERNEL_DRIVER
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#else
#include <windows.h>
#endif // CFB_KERNEL_DRIVER
// clang-format on

#define __STR(x) #x
#define STR(x) __STR(x)
#define __WIDE(x) L#x
#define WIDECHAR(x) __WIDE(x)
#define __WIDE2(x) L##x
#define WIDECHAR2(x) __WIDE2(x)
#define CONCAT(x, y) (x##y)

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

#define CFB_BROKER_DRIVER_SERVICE_NAME             CFB_DEVICE_NAME
#define CFB_BROKER_DRIVER_SERVICE_DESCRIPTION      L"IRP monitor driver for Canadian Furious Beaver broker"

#define CFB_BROKER_WIN32_SERVICE_NAME              L"CfbBrokerSvc"
#define CFB_BROKER_WIN32_SERVICE_DESCRIPTION       L"Service for having the CFB broker in background"

#ifndef countof
#define countof(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#endif

#ifndef boolstr
#define boolstr(x)                  ( (x) ? "true" : "false")
#endif

#ifndef boolstrw
#define boolstrw(x)                 ( (x) ? L"true" : L"false")
#endif

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b))
#endif // MIN

#ifndef MAX
#define MAX(a,b) ( (a) < (b) ? (b) : (a))
#endif // MAX

#ifndef USHRT_MAX
#define USHRT_MAX ((1 << (sizeof(u16) * 8)) - 1)
#endif // USHRT_MAX

#ifndef PLURAL_IF
#define PLURAL_IF(x) ((x) ? "s" : "")
#endif // PLURAL_IF

#ifndef WPLURAL_IF
#define WPLURAL_IF(x) ((x) ? L"s" : L"")
#endif // WPLURAL_IF

// clang-format on
