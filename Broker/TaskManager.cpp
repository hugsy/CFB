#include "TaskManager.h"


TaskManager::TaskManager()
{
	hPushEvent = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL
	);

};


TaskManager::~TaskManager()
{
};


void TaskManager::push(Task& t)
{
	_tasks.push(t);
	SetEvent(hPushEvent);
}


Task TaskManager::pop()
{
	Task t = _tasks.pop();
	return t;
}


