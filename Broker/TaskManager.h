#pragma once

#include "common.h"
#include "task.h"
#include "queue.h"


class TaskManager
{
public:
	TaskManager();
	TaskManager(std::string name);

	~TaskManager();

	void push(Task &t);
	Task pop();
	HANDLE GetPushEventHandle();
	BOOL SetName(std::string name);


private:
	HANDLE m_hPushEvent = INVALID_HANDLE_VALUE;
	Queue<Task> m_tasks;
	std::string m_name;
};