#pragma once

#define CFB_DEVICE_NAME L"\\Device\\CFB"
#define CFB_DEVICE_LINK L"\\??\\CFB"
#define CFB_DRIVER_TAG " BFC"


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
void DriverUnloadRoutine(PDRIVER_OBJECT DriverObject);
NTSTATUS DriverCreateCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp);
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information);
