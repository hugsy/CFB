#include "Task.h"

#define ToString(x) case x: return L# x


Task::Task(TaskType type, byte* data, uint32_t datalen, uint32_t code)
{
	_type = type;
	_state = Initialized;
	_code = code;
	_data_length = datalen;
	_data = new byte[datalen];
	::memcpy(_data, data, datalen);
}


Task::Task(TaskType type, byte* data, uint32_t datalen)
		: Task(type, data, datalen, 0)
{ }


Task::~Task()
{
	delete _data;
}


std::wstring Task::State()
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

std::wstring Task::TypeAsString()
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


TaskType Task::Type()
{
	return _type;
}


DWORD Task::IoctlCode()
{
	switch (_type)
	{
		case HookDriver: return IOCTL_AddDriver;
		case UnhookDriver: return IOCTL_RemoveDriver;
	}

	return ERROR_BAD_ARGUMENTS;
}


void Task::SetState(TaskState s)
{
	_state = s;
}


uint32_t Task::Length()
{
	return _data_length;
}


byte* Task::Data()
{
	return _data;
}
