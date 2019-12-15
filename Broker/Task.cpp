#include "Task.h"

#define ToString(x) case x: return L# x

static DWORD g_id = 0;
static std::mutex g_mutex;



Task::Task(const Task &t)
	:
	m_Type(t.m_Type),
	m_State(t.m_State),
	m_dwDataLength(t.m_dwDataLength),
	m_dwId(t.m_dwId),
	m_bIsRequest(t.m_bIsRequest),
	m_dwErrCode(t.m_dwErrCode)
{
	m_Data = new byte[t.m_dwDataLength];
	::memcpy(m_Data, t.m_Data, t.m_dwDataLength);
}


Task::Task(TaskType type, const byte* data, uint32_t datalen, uint32_t errcode, bool IsRequest)
	:
	m_Type(type), 
	m_State(TaskState::Initialized), 
	m_dwDataLength(datalen),
	m_bIsRequest(IsRequest),
	m_dwErrCode(errcode)
{
	m_Data = new byte[datalen];
	::memcpy(m_Data, data, datalen);

	std::lock_guard<std::mutex> guard(g_mutex);
	m_dwId = g_id++;
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
		ToString(TaskState::Initialized);
		ToString(TaskState::Queued);
		ToString(TaskState::Delivered);
		ToString(TaskState::Completed);
	}

	throw std::runtime_error("Unknown Task State");
}


const wchar_t* Task::TypeAsString()
{
	switch (m_Type)
	{
		ToString(TaskType::IoctlResponse);
		ToString(TaskType::HookDriver);
		ToString(TaskType::UnhookDriver);
		ToString(TaskType::GetDriverInfo);
		ToString(TaskType::GetNumberOfDriver);
		ToString(TaskType::NotifyEventHandle);
		ToString(TaskType::EnableMonitoring);
		ToString(TaskType::DisableMonitoring);
		ToString(TaskType::GetInterceptedIrps);
		ToString(TaskType::ReplayIrp);
		ToString(TaskType::StoreTestCase);
		ToString(TaskType::EnumerateDrivers);
	}

	dbg(L"Undeclared TaskType %d\n", m_Type);
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


const DWORD Task::ErrCode()
{
	if (m_bIsRequest)
		return -1;

	return m_dwErrCode;
}
