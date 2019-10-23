#include "TaskManager.h"


TaskManager::TaskManager()
{
	m_hPushEvent = CreateEvent(NULL, FALSE,	FALSE, NULL);

	if(!m_hPushEvent)
		throw std::runtime_error("CreateEvent(hPushEvent) failed");

}



TaskManager::~TaskManager()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"deleting TaskManager '%s'\n", m_name.c_str());
#endif // _DEBUG

	CloseHandle(m_hPushEvent);
	m_hPushEvent = INVALID_HANDLE_VALUE;
}


void TaskManager::push(Task task)
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"%s pushes '%s' task (id=%d)\n", m_name.c_str(), task.TypeAsString(), task.Id());
#endif // _DEBUG

	//
	// push a copy of the task to the queue
	// 
	std::unique_lock<std::mutex> mlock(m_mutex);
	m_task_queue.push(task);
	mlock.unlock();
	m_cond.notify_one();

	//
	// notify other threads an item has been pushed
	//
	SetEvent(m_hPushEvent);
	task.SetState(TaskState::Queued);
}


Task TaskManager::pop()
{
	std::unique_lock<std::mutex> mlock(m_mutex);
	
	while (m_task_queue.empty())
		m_cond.wait(mlock);
	

	//
	// copy-pop the top task, mark as delivered
	//
	Task t(m_task_queue.front());
	m_task_queue.pop();

	t.SetState(TaskState::Delivered);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"%s pops '%s' task ID=%d\n", m_name.c_str(), t.TypeAsString(), t.Id());
#endif // _DEBUG

	return t;
}



BOOL TaskManager::SetName(const std::wstring name)
{
	m_name = name;

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"setting new name %s\n", m_name.c_str());
#endif // _DEBUG

	return TRUE;
}