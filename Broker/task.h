#pragma once

#include "common.h"

#include <string>

enum TaskType : uint32_t
{
	TaskTypeMin = 0,
	IoctlResponse,
	HookDriver,
	UnhookDriver,
	GetDriverInfo,
	GetNumberOfDriver,
	TaskTypeMax
};


enum TaskState : uint16_t
{
	Initialized,
	Queued,
	Delivered,
	Completed
};

#define ToString(x) case x: return L# x

class Task
{
public:

	Task(TaskType type, byte* data, uint32_t datalen, uint32_t code)
	{
		_type = type;
		_state = Initialized;
		_code = code;
		_data_length = datalen;
		_data = new byte[datalen];
		::memcpy(_data, data, datalen);
	}

	Task(TaskType type, byte* data, uint32_t datalen)
		: Task(type, data, datalen, 0)
	{ }

	~Task()
	{
		delete _data;
	}

	std::wstring State() 
	{
		switch (_state)
		{
			ToString(Initialized);
			ToString(Queued);
			ToString(Delivered);
			ToString(Completed);
		}
		return L"??";
	}

	std::wstring TypeAsString()
	{
		switch (_type)
		{
			ToString(HookDriver);
			ToString(UnhookDriver);
			ToString(GetDriverInfo);
			ToString(GetNumberOfDriver);
		}
		return L"(Unknown)";
	}

	TaskType Type() 
	{ 
		return _type; 
	}

	DWORD IoctlCode()
	{
		switch (_type)
		{
		case HookDriver: return IOCTL_AddDriver;
		case UnhookDriver: return IOCTL_RemoveDriver;
		}

		return ERROR_BAD_ARGUMENTS;
	}

	void SetState(TaskState s)
	{
		_state = s;
	}

	uint32_t Length()
	{
		return _data_length;
	}

	byte* Data()
	{
		return _data;
	}



private:
	TaskType _type;
	TaskState _state;
	byte* _data = NULL;
	uint32_t _data_length;
	DWORD _code;
};