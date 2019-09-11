#pragma once

#include "common.h"
#include "task.h"
#include "queue.h"


class TaskManager
{
public:
	TaskManager();
	~TaskManager();
	void push(Task &t);
	Task pop();

private:
	Queue<Task> _tasks;
};