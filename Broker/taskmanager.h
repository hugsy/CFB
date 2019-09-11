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
		_tasks->push(t);
	}

	Task pop()
	{
		return _tasks->pop();
	}

private:
	Queue<Task> * _tasks;
};


