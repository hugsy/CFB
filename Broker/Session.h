#pragma once

#include "common.h"
#include "taskmanager.h"
#include "ServiceManager.h"
#include "FrontEndServer.h"
#include "Irp.h"

#include <queue>

enum SessionState : uint16_t
{
	Idle,
	Running
};

class Session
{
public:
	Session();
	~Session();


	void Start();
	void Stop();
	BOOL IsRunning();


	//
	// The ServiceManager is responsible for managing the IrpDumper driver
	//
	ServiceManager ServiceManager;

	//
	// The FrontEndServer handles the communication with the frontend
	//
	FrontEndServer FrontEndServer;

	//
	// Those 2 task managers are used for dispatching requests from/to the frontend
	//
	TaskManager RequestTasks;
	TaskManager ResponseTasks;
	

	//
	// The termination Event: if set, everything must stop
	//
	HANDLE m_hTerminationEvent = INVALID_HANDLE_VALUE;


	//
	// Handle between frontend (GUI, TUI) and the broker
	// For now this is done with NamedPipes
	//
	HANDLE m_hFrontendThreadHandle = INVALID_HANDLE_VALUE;


	//
	// Handle between the broker and the IrpDumper driver
	// 
	HANDLE m_hBackendThreadHandle = INVALID_HANDLE_VALUE;


	//
	// This queue receives all the IRP intercepted by driver.
	//
	std::queue<Irp> m_IrpQueue;
	std::mutex m_IrpMutex;

private:
	//
	// The global state of the current session
	//
	SessionState m_State = SessionState::Idle;
};
