#pragma once


#include "Common.h"

#include "IoctlCodes.h"
#include "Utils.h"
#include "HookedDrivers.h"
#include "InterceptedIrpHandler.h"
#include "Queue.h"

#include "IoAddDriver.h"
#include "IoRemoveDriver.h"
#include "IoGetDriverInfo.h"
#include "IoEnableDisableMonitoring.h"
#include "IoStoreTestCase.h"


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp );
VOID _Function_class_(DRIVER_UNLOAD) DriverUnloadRoutine(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCreateRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverDeviceControlRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp);
NTSTATUS CompleteRequest(_In_ PIRP Irp, _In_ NTSTATUS status, _In_ ULONG_PTR Information);
NTSTATUS _Function_class_(DRIVER_DISPATCH) IrpNotImplementedHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp );




//
// See https://lylone.wordpress.com/2007/05/08/obreferenceobjectbynameundocumented/
//
extern POBJECT_TYPE* IoDriverObjectType;
extern POBJECT_TYPE* IoDeviceObjectType;

extern NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
	_In_ PUNICODE_STRING ObjectPath,
	_In_ ULONG Attributes,
	_In_opt_ PACCESS_STATE PassedAccessState,
	_In_opt_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_TYPE ObjectType,
	_In_ KPROCESSOR_MODE AccessMode,
	_Inout_opt_  PVOID ParseContext,
	_Out_ PVOID *ObjectPtr
);

//
// Generic interception routine
//
NTSTATUS InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

BOOLEAN InterceptGenericFastIoRead(
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER FileOffset,
	IN ULONG Length,
	IN BOOLEAN Wait,
	IN ULONG LockKey,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN InterceptGenericFastIoWrite(
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER FileOffset,
	IN ULONG Length,
	IN BOOLEAN Wait,
	IN ULONG LockKey,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN InterceptGenericFastIoDeviceControl(
	IN PFILE_OBJECT FileObject,
	IN BOOLEAN Wait,
	IN PVOID InputBuffer OPTIONAL,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer OPTIONAL,
	IN ULONG OutputBufferLength,
	IN ULONG IoControlCode,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS EnableMonitoring();
NTSTATUS DisableMonitoring();
BOOLEAN IsMonitoringEnabled();
