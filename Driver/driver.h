#include "../Common/common.h"

/*
#define CFB_DEVICE_NAME		L"\\Device\\CFB"
#define CFB_DEVICE_LINK		L"\\??\\CFB"
*/
#define CFB_MAX_DEVICES		32
#define CFB_DEVICE_TAG		0x20424643 // " BFC"


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnloadRoutine(PDRIVER_OBJECT DriverObject);
NTSTATUS DriverCreateCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information);


/* https://lylone.wordpress.com/2007/05/08/obreferenceobjectbynameundocumented/ */
extern POBJECT_TYPE* IoDriverObjectType;

extern NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
	IN PUNICODE_STRING ObjectPath,
	IN ULONG Attributes,
	IN PACCESS_STATE PassedAccessState OPTIONAL,
	IN ACCESS_MASK DesiredAccess OPTIONAL,
	IN POBJECT_TYPE ObjectType,
	IN KPROCESSOR_MODE AccessMode,
	IN OUT PVOID ParseContext OPTIONAL,
	OUT PVOID *ObjectPtr
);

#define HOOKED_DRIVER_MAX_NAME_LEN 512

typedef struct __hooked_driver
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
	PVOID OldDeviceControlRoutine;
	struct __hooked_driver *Next;
}
HOOKED_DRIVER, *PHOOKED_DRIVER;

PHOOKED_DRIVER g_HookedDriversHead;

NTSTATUS DummyHookedDispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);