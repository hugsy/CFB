#pragma once

#include "common.h"
#include "Task.h"

#include <queue>
#include <mutex>

class TaskManager
{
public:
	TaskManager();
	~TaskManager();

	void push(Task t);
	Task pop();
	BOOL SetName(const std::wstring name);

	HANDLE m_hPushEvent = INVALID_HANDLE_VALUE;


private:
	std::wstring m_name;
	std::queue<Task> m_task_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond;
	
};