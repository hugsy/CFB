#pragma once

#include "common.h"

#include <string>
#include <mutex>
#include <map>


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
	IoctlResponse,
	HookDriver,
	UnhookDriver,
	GetDriverInfo,
	GetNumberOfDriver,
	NotifyEventHandle,
	EnableMonitoring,
	DisableMonitoring,
	StoreTestCase,
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

	Task(TaskType type, byte* data, uint32_t datalen, uint32_t code);
	Task(TaskType type, byte* data, uint32_t datalen);
	~Task();

	std::wstring State();
	std::wstring TypeAsString();
	TaskType Type();
	DWORD IoctlCode();
	void SetState(TaskState s);
	uint32_t Length();
	byte* Data();
	DWORD Id();


private:
	TaskType m_Type;
	TaskState m_State;
	byte* m_Data = NULL;
	uint32_t m_dwDataLength;
	DWORD m_dwIoctlCode;
	DWORD m_dwId;
};