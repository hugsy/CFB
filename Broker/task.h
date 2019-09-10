#pragma once

#include "common.h"

#include <string>

enum TaskType : short
{
	HookDriver,
	UnhookDriver,
	GetDriverInfo,
	GetNumberOfDriver
};


enum TaskState : short
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
	
	Task(TaskType type, byte* data, size_t datalen)
	{
		_type = type;
		_state = Initialized;
		_data = new byte[datalen];
		::memcpy(_data, data, datalen);
	}

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

	std::wstring Type()
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

	void SetState(TaskState s)
	{
		_state = s;
	}


private:
	TaskType _type;
	TaskState _state;
	byte* _data = NULL;
};