#include "Session.h"


Session::Session()
{
	//
	// Initial state has to be idle
	//
	State = GLOBAL_STATE_IDLE;


	//
	// Create the main event to stop the running threads
	//
	hTerminationEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if(!hTerminationEvent)
		throw std::runtime_error("CreateEvent(hTerminationEvent) failed");

	//
	// Defining some attributes
	//
	RequestTasks.SetName(L"RequestTaskManager");
	ResponseTasks.SetName(L"ResponseTaskManager");
}


Session::~Session()
{
	State = GLOBAL_STATE_IDLE;

	CloseHandle(hTerminationEvent);

	if (hFrontendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(hFrontendThreadHandle);

	if (hBackendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(hBackendThreadHandle);
}


void Session::Start()
{
	State = GLOBAL_STATE_RUNNING;
}


void Session::Stop()
{
	State = GLOBAL_STATE_IDLE;
	SetEvent(hTerminationEvent);
}


BOOL Session::IsRunning()
{
	return State == GLOBAL_STATE_RUNNING;
}