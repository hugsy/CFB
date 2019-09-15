#pragma once

#include "common.h"

#include <string>
#include <mutex>

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
	TaskTypeMax
};


enum TaskState : uint16_t
{
	Initialized,
	Queued,
	Delivered,
	Completed
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
	TaskType _type;
	TaskState _state;
	byte* _data = NULL;
	uint32_t _data_length;
	DWORD _code;
	DWORD _id;
};