#include "Session.h"


Session::Session()
{
	//
	// Initial state has to be idle
	//
	m_State = GLOBAL_STATE_IDLE;


	//
	// Create the main event to stop the running threads
	//
	m_hTerminationEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if(!m_hTerminationEvent)
		throw std::runtime_error("CreateEvent(hTerminationEvent) failed");

	//
	// Defining some attributes
	//
	RequestTasks.SetName(L"RequestTaskManager");
	ResponseTasks.SetName(L"ResponseTaskManager");
}


Session::~Session()
{
	m_State = GLOBAL_STATE_IDLE;

	CloseHandle(m_hTerminationEvent);

	if (m_hFrontendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFrontendThreadHandle);

	if (m_hBackendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_hBackendThreadHandle);
}


void Session::Start()
{
	m_State = GLOBAL_STATE_RUNNING;
}


void Session::Stop()
{
	m_State = GLOBAL_STATE_IDLE;
	SetEvent(m_hTerminationEvent);
}


BOOL Session::IsRunning()
{
	return m_State == GLOBAL_STATE_RUNNING;
}