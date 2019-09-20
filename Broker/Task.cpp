#include "Task.h"

#define ToString(x) case x: return L# x

static DWORD g_id = 0;
static std::mutex g_mutex;


Task::Task(TaskType type, byte* data, uint32_t datalen)
{
	std::lock_guard<std::mutex> guard(g_mutex);
	m_dwId = g_id++;
	m_Type = type;
	m_State = TaskState::Initialized;
	m_dwDataLength = datalen;
	m_Data = new byte[datalen];
	::memcpy(m_Data, data, datalen);
}



Task::~Task()
{
	//
	// the buffer associated with the Task can only be freeed when the 
	// task is completed.
	//
	if (m_State == TaskState::Completed)
	{
		delete[] m_Data;
	}
}


const wchar_t* Task::StateAsString()
{
	switch (m_State)
	{
		ToString(Initialized);
		ToString(Queued);
		ToString(Delivered);
		ToString(Completed);
	}

	throw std::runtime_error("Unknown Task State");
}


const wchar_t* Task::TypeAsString()
{
	switch (m_Type)
	{
		ToString(IoctlResponse);
		ToString(HookDriver);
		ToString(UnhookDriver);
		ToString(GetDriverInfo);
		ToString(GetNumberOfDriver);
		ToString(NotifyEventHandle);
		ToString(EnableMonitoring);
		ToString(DisableMonitoring);
		ToString(GetInterceptedIrps);
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, "Undeclared TaskType %d\n", m_Type);
#endif // _DEBUG

	return L"(UnknownType)";
}


const TaskType Task::Type()
{
	return m_Type;
}


const DWORD Task::IoctlCode()
{
	std::map<TaskType, DWORD>::iterator it = g_TaskIoctls.find(m_Type);
	if(it == g_TaskIoctls.end())
		return (DWORD)-1;

	return it->second;
}


void Task::SetState(TaskState s)
{
	m_State = s;
}


const uint32_t Task::Length()
{
	return m_dwDataLength;
}


byte* Task::Data()
{
	return m_Data;
}


const DWORD Task::Id()
{
	return m_dwId;
}
