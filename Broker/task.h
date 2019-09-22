#pragma once

#include "common.h"

#include <string>
#include <mutex>
#include <map>
#include <vector>


enum TaskState : uint16_t
{
	Initialized,
	Queued,
	Delivered,
	Completed
};


enum TaskType : uint32_t
{
	TaskTypeMin = 0,
	IoctlResponse = 1,
	HookDriver,
	UnhookDriver,
	GetDriverInfo,
	GetNumberOfDriver = 5,
	NotifyEventHandle,
	EnableMonitoring,
	DisableMonitoring,
	GetInterceptedIrps,
	StoreTestCase = 10,
	TaskTypeMax
};


static std::map<TaskType, DWORD> g_TaskIoctls = 
{
	{HookDriver, IOCTL_AddDriver},
	{UnhookDriver, IOCTL_RemoveDriver},
	{GetNumberOfDriver, IOCTL_GetNumberOfDrivers},
	{GetDriverInfo, IOCTL_ListAllDrivers},
	{NotifyEventHandle, IOCTL_SetEventPointer},
	{EnableMonitoring, IOCTL_EnableMonitoring},
	{DisableMonitoring, IOCTL_DisableMonitoring},
	{StoreTestCase, IOCTL_StoreTestCase},
};




class Task
{
public:

	Task(TaskType type, byte* data, uint32_t datalen);
	Task(HANDLE Handle);
	~Task();

	const wchar_t* StateAsString();
	const wchar_t* TypeAsString();
	const TaskType Type();
	const DWORD IoctlCode();
	void SetState(TaskState s);
	const uint32_t Length();
	byte* Data();
	const DWORD Id();
	const byte* AsTlv();


private:
	TaskType m_Type;
	TaskState m_State;
	byte* m_Data = nullptr;
	uint32_t m_dwDataLength;
	DWORD m_dwId;
};