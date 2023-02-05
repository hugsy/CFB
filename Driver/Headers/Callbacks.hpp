#pragma once


#include "Common.hpp"

namespace CFB::Driver::Callbacks
{
///
/// @brief This is the main interception routine: it will find the HookedDriver  associated to a DeviceObject. If
/// any is found, and capture mode is enabled the IRP data will be pushed to the queue of captured data.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
NTSTATUS
InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

///
/// @brief
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
NTSTATUS
InterceptedDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

///
/// @brief The ReadFile() interception routine wrapper.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS STATUS_SUCCESS on success.
///
NTSTATUS
InterceptedReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

///
/// @brief The WriteFile() interception routine wrapper.
///
/// @param DeviceObject
/// @param Irp
/// @return NTSTATUS
///
NTSTATUS
InterceptedWriteRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

///
///@brief
///
///@param DeviceObject
///@param Type
///@param Buffer
///@param BufferLength
///@param IoControlCode
///@param Flags
///@param pIrpOut
///@return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoRoutine(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ UINT32 Type,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _In_ ULONG IoControlCode,
    _In_ UINT32 Flags,
    _Inout_ PVOID* pIrpOut);

///
/// @brief The `InterceptGenericFastIoDeviceControl()` interception routine wrapper.
///
/// @param FileObject
/// @param Wait
/// @param InputBuffer
/// @param InputBufferLength
/// @param OutputBuffer
/// @param OutputBufferLength
/// @param IoControlCode
/// @param IoStatus
/// @param DeviceObject
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoDeviceControl(
    _In_ PFILE_OBJECT FileObject,
    _In_ BOOLEAN Wait,
    _In_opt_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_opt_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _In_ ULONG IoControlCode,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject);

///
/// @brief The `InterceptGenericFastIoRead()` interception routine wrapper.
///
/// @param FileObject
/// @param FileOffset
/// @param BufferLength
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
///
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoRead(
    _In_ PFILE_OBJECT FileObject,
    _In_ PLARGE_INTEGER FileOffset,
    _In_ ULONG Length,
    _In_ BOOLEAN Wait,
    _In_ ULONG LockKey,
    _Out_ PVOID Buffer,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject);

///
/// @brief The InterceptGenericFastIoWrite() interception routine wrapper.
///
/// @param FileObject
/// @param FileOffset
/// @param Length
/// @param Wait
/// @param LockKey
/// @param Buffer
/// @param IoStatus
/// @param DeviceObject
///
/// @return BOOLEAN
///
BOOLEAN
InterceptGenericFastIoWrite(
    _In_ PFILE_OBJECT FileObject,
    _In_ PLARGE_INTEGER FileOffset,
    _In_ ULONG Length,
    _In_ BOOLEAN Wait,
    _In_ ULONG LockKey,
    _Out_ PVOID Buffer,
    _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ PDEVICE_OBJECT DeviceObject);
} // namespace CFB::Driver::Callbacks
