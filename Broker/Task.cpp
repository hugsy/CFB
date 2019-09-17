#include "Task.h"

#define ToString(x) case x: return L# x

static DWORD g_id = 0;
static std::mutex g_mutex;


Task::Task(TaskType type, byte* data, uint32_t datalen, uint32_t code)
{
	std::lock_guard<std::mutex> guard(g_mutex);
	m_dwId = g_id++;
	m_Type = type;
	m_State = Initialized;
	m_dwIoctlCode = code;
	m_dwDataLength = datalen;
	m_Data = new byte[datalen];
	::memcpy(m_Data, data, datalen);
}


Task::Task(TaskType type, byte* data, uint32_t datalen)
		: Task(type, data, datalen, 0)
{ 

}

/*
Task& Task::operator=(const Task& t)
{
	byte* pData = m_Data;
	m_Data = new byte[t.m_dwDataLength];
	::memcpy(m_Data, t.m_Data, t.m_dwDataLength);
	delete[] pData;
	return *this;
}
*/

Task::~Task()
{
	delete[] m_Data;
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
	return L"??";
}


const wchar_t* Task::TypeAsString()
{
	switch (m_Type)
	{
		ToString(HookDriver);
		ToString(UnhookDriver);
		ToString(GetDriverInfo);
		ToString(GetNumberOfDriver);
	}
	return L"(Unknown)";
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
