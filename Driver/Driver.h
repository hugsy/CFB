#pragma once

#include "Common.h"
#include "../Common/common.h"

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



NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverReadRoutine( PDEVICE_OBJECT DeviceObject, PIRP Irp );
VOID _Function_class_(DRIVER_UNLOAD) DriverUnloadRoutine(PDRIVER_OBJECT DriverObject);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCreateRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information);
NTSTATUS _Function_class_(DRIVER_DISPATCH) IrpNotImplementedHandler( PDEVICE_OBJECT DeviceObject, PIRP Irp );




//
// See https://lylone.wordpress.com/2007/05/08/obreferenceobjectbynameundocumented/
//
extern POBJECT_TYPE* IoDriverObjectType;
extern POBJECT_TYPE* IoDeviceObjectType;

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

//
// Generic interception routine
//
NTSTATUS InterceptGenericRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);

BOOLEAN InterceptGenericFastIoDeviceControl(
	PFILE_OBJECT FileObject,
	BOOLEAN Wait,
	IN PVOID InputBuffer OPTIONAL,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer OPTIONAL,
	IN ULONG OutputBufferLength,
	IN ULONG IoControlCode,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
);

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


NTSTATUS EnableMonitoring();
NTSTATUS DisableMonitoring();
BOOLEAN IsMonitoringEnabled();
