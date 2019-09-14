#include "TaskManager.h"


TaskManager::TaskManager(std::string name)
{
	m_hPushEvent = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL
	);

	if(!m_hPushEvent)
		throw std::runtime_error("CreateEvent(hPushEvent) failed");

	SetName(name);
}


TaskManager::TaskManager()
	: TaskManager::TaskManager("default")
{
}


TaskManager::~TaskManager()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"deleting TM '%s'\n", m_name);
#endif // _DEBUG

	CloseHandle(m_hPushEvent);
}


void TaskManager::push(Task& t)
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"pushing new task of type='%s' to %s\n", t.TypeAsString(), m_name);
#endif // _DEBUG

	m_tasks.push(t);
	SetEvent(m_hPushEvent);
	t.SetState(TaskState::Queued);
}


Task TaskManager::pop()
{
	Task t = m_tasks.pop();
	t.SetState(TaskState::Delivered);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"pop task of type='%s' to %s\n", t.TypeAsString(), m_name);
#endif // _DEBUG

	return t;
}


HANDLE TaskManager::GetPushEventHandle()
{
	return m_hPushEvent;
}


BOOL TaskManager::SetName(std::string name)
{
	m_name = name;
	return TRUE;
}