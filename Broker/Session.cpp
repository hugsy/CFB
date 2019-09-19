#include "Session.h"


Session::Session()
{
	//
	// Initial state has to be idle
	//
	m_State = SessionState::Idle;


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
	m_State = SessionState::Idle;

	CloseHandle(m_hTerminationEvent);

	if (m_hFrontendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFrontendThreadHandle);

	if (m_hBackendThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_hBackendThreadHandle);
}


void Session::Start()
{
	m_State = SessionState::Running;
}


void Session::Stop()
{
	m_State = SessionState::Idle;
	SetEvent(m_hTerminationEvent);
}


BOOL Session::IsRunning()
{
	return m_State == SessionState::Running;
}