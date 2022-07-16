#ifdef CFB_KERNEL_DRIVER
#include <ntifs.h>
#include <wdm.h>
#include <ntddk.h>
#else
#include <windows.h>
#endif // CFB_KERNEL_DRIVER

//
// Types
//

///
/// Static types
///
using u8 = UINT8;
using u16 = UINT16;
using u32 = UINT32;
using u64 = UINT64;

using i8 = INT8;
using i16 = INT16;
using i32 = INT32;
using i64 = INT64;

using usize = SIZE_T;

#ifndef countof
#define countof(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#endif

#define CFB_DEVICE_NAME             L"IrpMonitor"
#define CFB_DEVICE_PATH             L"\\Device\\" CFB_DEVICE_NAME
#define CFB_DOS_DEVICE_PATH         L"\\??\\" CFB_DEVICE_NAME
#define CFB_USER_DEVICE_PATH        L"\\\\.\\" CFB_DEVICE_NAME

#define CFB_DEVICE_TAG				'CFB '

#ifndef boolstr
#define boolstr(x)                  ( (x) ? "true" : "false")
#endif

#ifndef boolstrw
#define boolstrw(x)                 ( (x) ? L"true" : L"false")
#endif

