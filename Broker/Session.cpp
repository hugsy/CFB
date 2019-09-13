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

}


Session::~Session()
{
	State = GLOBAL_STATE_IDLE;
	CloseHandle(hTerminationEvent);
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