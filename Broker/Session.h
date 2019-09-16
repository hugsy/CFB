#pragma once

#include "common.h"
#include "taskmanager.h"
#include "ServiceManager.h"
#include "FrontEndServer.h"



#define GLOBAL_STATE_IDLE 0
#define GLOBAL_STATE_RUNNING  1


class Session
{
public:
	Session();
	~Session();


	void Start();
	void Stop();
	BOOL IsRunning();


	ServiceManager ServiceManager;
	FrontEndServer FrontEndServer;
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


	

private:
	//
	// 0 -> not running
	// 1 -> running
	//
	uint16_t m_State = GLOBAL_STATE_IDLE;

	

};