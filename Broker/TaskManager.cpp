#include "TaskManager.h"


TaskManager::TaskManager()
{
};


TaskManager::~TaskManager()
{
};


void TaskManager::push(Task& t)
{
	_tasks.push(t);
}


Task TaskManager::pop()
{
	return _tasks.pop();
}


