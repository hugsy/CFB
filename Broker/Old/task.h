#pragma once

#include "common.h"

#include <string>
#include <mutex>
#include <map>
#include <vector>
#include <array>
#include <cstddef>


enum class TaskState : uint16_t
{
	Initialized,
	Queued,
	Delivered,
	Completed
};


enum class TaskType : uint32_t
{
	TaskTypeMin = 0,
	GetOsInfo,
	HookDriver,
	UnhookDriver,
	GetDriverInfo,
	GetNumberOfDriver,
	NotifyEventHandle,
	EnableMonitoring,
	DisableMonitoring,
	GetInterceptedIrps,
	ReplayIrp,
	StoreTestCase,
	EnumerateDrivers,
	EnableDriver,
	DisableDriver,
	GetNamesOfHookedDrivers,
	TaskTypeMax = 16
};


static std::map<TaskType, DWORD> g_TaskIoctls = 
{
	{TaskType::HookDriver,                IOCTL_AddDriver},
	{TaskType::UnhookDriver,              IOCTL_RemoveDriver},
	{TaskType::GetNumberOfDriver,         IOCTL_GetNumberOfDrivers},
	{TaskType::GetNamesOfHookedDrivers,   IOCTL_GetNamesOfDrivers},
	{TaskType::GetDriverInfo,             IOCTL_GetDriverInfo},
	{TaskType::NotifyEventHandle,         IOCTL_SetEventPointer},
	{TaskType::EnableMonitoring,          IOCTL_EnableMonitoring},
	{TaskType::DisableMonitoring,         IOCTL_DisableMonitoring},
	{TaskType::StoreTestCase,             IOCTL_StoreTestCase},
	{TaskType::EnableDriver,              IOCTL_EnableDriver},
	{TaskType::DisableDriver,             IOCTL_DisableDriver},
};




class Task
{
public:
	Task(const Task& t);
	Task(TaskType type, const byte* data, uint32_t datalen, uint32_t errcode, bool IsRequest);
	~Task();

	const wchar_t* StateAsString();
	const wchar_t* TypeAsString();
	const TaskType Type();
	const DWORD IoctlCode();
	void SetState(TaskState s);
	const uint32_t Length();
	byte* Data();
	const DWORD Id();
	const DWORD ErrCode();


private:
	TaskType m_Type;
	TaskState m_State;
	byte* m_Data = nullptr;
	uint32_t m_dwDataLength;
	DWORD m_dwId;
	DWORD m_dwErrCode;
	bool m_bIsRequest;
};