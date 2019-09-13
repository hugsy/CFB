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
	HANDLE GetPushEventHandle();


private:
	HANDLE hPushEvent = INVALID_HANDLE_VALUE;
	Queue<Task> _tasks;
};