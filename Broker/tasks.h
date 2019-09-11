#pragma once

#include "common.h"
#include "task.h"
#include "queue.h"

class TaskManager
{
public:
	TaskManager() 
	{
		_tasks = new Queue<Task>();
	};

	~TaskManager() 
	{
		delete _tasks;
	};

	void push(Task t)
	{
		_tasks.push(t);
	}

private:
	//static Queue<Task> g_RequestTasks = {};
	//static Queue<Task> g_ResponseTasks = {};
	Queue<Task> * _tasks;
};
