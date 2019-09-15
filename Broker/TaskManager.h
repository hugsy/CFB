#pragma once

#include "common.h"
#include "task.h"
#include "queue.h"


class TaskManager
{
public:
	TaskManager();
	TaskManager(std::wstring name);

	~TaskManager();

	void push(Task &t);
	Task pop();
	HANDLE GetPushEventHandle();
	BOOL SetName(std::wstring name);


private:
	HANDLE m_hPushEvent = INVALID_HANDLE_VALUE;
	Queue<Task> m_tasks;
	std::wstring m_name;
};