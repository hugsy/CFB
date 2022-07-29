#include "Session.h"


Session::Session()
	: FrontEndServer(*this)
{
	//
	// Initial state has to be idle
	//
	m_State = SessionState::Idle;


	//
	// Create the main event to stop the running threads
	//
	m_hTerminationEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if(!m_hTerminationEvent)
		RAISE_GENERIC_EXCEPTION("CreateEvent(hTerminationEvent) failed");


	//
	// Defining some attributes
	//
	RequestTasks.SetName(L"RequestTaskManager");
	ResponseTasks.SetName(L"ResponseTaskManager");
}


Session::~Session()
{
	m_State = SessionState::Idle;
	::ResetEvent(m_hTerminationEvent);

	::CloseHandle(m_hTerminationEvent);

	if (m_hFrontendThread != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFrontendThread);

	if (m_hBackendThread != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hBackendThread);
}


void Session::Start()
{
	m_State = SessionState::Running;
}


void Session::Stop()
{
	m_State = SessionState::Idle;
	::ResetEvent(m_hTerminationEvent);
	::SetEvent(m_hTerminationEvent);
}


BOOL Session::IsRunning()
{
	return m_State == SessionState::Running;
}