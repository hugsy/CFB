#include "TaskManager.h"


TaskManager::TaskManager()
{
	hPushEvent = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL
	);

	if(!hPushEvent)
		throw std::runtime_error("CreateEvent(hPushEvent) failed");

};


TaskManager::~TaskManager()
{
	CloseHandle(hPushEvent);
};


void TaskManager::push(Task& t)
{
	_tasks.push(t);
	SetEvent(hPushEvent);
	t.SetState(Queued);
}


Task TaskManager::pop()
{
	Task t = _tasks.pop(); 
	t.SetState(Delivered);
	return t;
}


HANDLE TaskManager::GetPushEventHandle()
{
	return hPushEvent;
}