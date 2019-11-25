#include "PipeTransportManager.h"
#include "Session.h"

#include <aclapi.h>
#include <psapi.h>

#include "json.hpp"
using json = nlohmann::json;


/*++
Routine Description:

Create the named pipe responsible for the communication with the GUI. To do, a Security
Descriptor is created with Explicit Access set for Everyone (including remote clients),
to Read/Write the pipe.

Therefore, we must be careful about that as any user would be able to send some commands
to the broker pipe (and therefore to the kernel driver).


Arguments:

	None


Return Value:
	Returns TRUE upon successful creation of the pipe, FALSE if any error occured.

--*/
BOOL PipeTransportManager::CreatePipe()
{
	BOOL fSuccess = FALSE;
	SID_IDENTIFIER_AUTHORITY SidAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	EXPLICIT_ACCESS ea[1] = { 0 };
	PACL pNewAcl = NULL;
	PSID pEveryoneSid = NULL;
	SECURITY_ATTRIBUTES SecurityAttributes = { 0 };
	SECURITY_DESCRIPTOR SecurityDescriptor = { 0 };

	do
	{
		dbg(L"Defining new SD for pipe\n");

		//
		// For now, SD is set for Everyone to have RW access 
		//

		if (!::AllocateAndInitializeSid(
			&SidAuthWorld,
			1,
			SECURITY_WORLD_RID,
			0, 0, 0, 0, 0, 0, 0,
			&pEveryoneSid
		)
			)
		{
			PrintErrorWithFunctionName(L"AllocateAndInitializeSid");
			fSuccess = FALSE;
			break;
		}


		//
		// Populate the EA entry
		//
		ea[0].grfAccessPermissions = GENERIC_ALL;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSid;


		//
		// Apply the EA to the ACL
		//
		if (::SetEntriesInAcl(1, ea, NULL, &pNewAcl) != ERROR_SUCCESS)
		{
			PrintErrorWithFunctionName(L"SetEntriesInAcl");
			fSuccess = FALSE;
			break;
		}


		//
		// Set the SD to new ACL
		//
		if (!::InitializeSecurityDescriptor(&SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
		{
			PrintErrorWithFunctionName(L"InitializeSecurityDescriptor");
			fSuccess = FALSE;
			break;
		}

		if (!::SetSecurityDescriptorDacl(&SecurityDescriptor, TRUE, pNewAcl, FALSE))
		{
			PrintErrorWithFunctionName(L"SetSecurityDescriptorDacl");
			fSuccess = FALSE;
			break;
		}


		//
		// Init the SA
		//
		SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
		SecurityAttributes.bInheritHandle = FALSE;


#ifdef _DEBUG
		xlog(LOG_DEBUG, L"Creating named pipe '%s'...\n", CFB_PIPE_NAME);
#endif

		//
		// create the overlapped pipe
		//
		HANDLE hServer = ::CreateNamedPipe(
			CFB_PIPE_NAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_ACCEPT_REMOTE_CLIENTS | PIPE_WAIT,
			CFB_PIPE_MAXCLIENTS,
			CFB_PIPE_INBUFLEN,
			CFB_PIPE_OUTBUFLEN,
			0,
			&SecurityAttributes
		);

		if (hServer == INVALID_HANDLE_VALUE)
		{
			PrintErrorWithFunctionName(L"CreateNamedPipe()");
			fSuccess = FALSE;
			break;
		}

		m_hServer = hServer;

		m_oOverlap.hEvent = ::CreateEvent(nullptr, FALSE, TRUE, nullptr);
		if (!m_oOverlap.hEvent)
		{
			xlog(LOG_CRITICAL, L"failed to create an event object for frontend thread\n");
			return false;
		}


		//
		// last, connect to the named pipe
		//
		fSuccess = ConnectPipe();
	} while (false);

	if (pEveryoneSid)
		FreeSid(pEveryoneSid);

	if (pNewAcl)
		LocalFree(pNewAcl);


	return fSuccess;
}





/*++

Routine Description:

Flush all the data and close the pipe.


Arguments:

	None


Return Value:
	Returns TRUE upon successful termination of the pipe, FALSE if any error occured.

--*/
BOOL PipeTransportManager::ClosePipe()
{
	dbg(L"Closing NamedPipe '%s'...\n", CFB_PIPE_NAME);


	BOOL fSuccess = TRUE;

	do
	{
		if (m_hServer == INVALID_HANDLE_VALUE)
		{
			//
			// already closed
			//
			break;
		}

		if (m_dwServerState != ServerState::Disconnected)
		{
			//
			// Wait until all data was consumed
			//
			if (!::FlushFileBuffers(m_hServer))
			{
				PrintErrorWithFunctionName(L"FlushFileBuffers()");
				fSuccess = FALSE;
			}

			//
			// Then close down the named pipe
			//
			if (!::DisconnectNamedPipe(m_hServer))
			{
				PrintErrorWithFunctionName(L"DisconnectNamedPipe()");
				fSuccess = FALSE;
			}
		}

		fSuccess = ::CloseHandle(m_hServer);
		m_hServer = INVALID_HANDLE_VALUE;
	} while (false);

	return fSuccess;
}


/*++

Routine Description:


Arguments:


Return Value:

--*/
BOOL PipeTransportManager::DisconnectAndReconnect()
{
	if (!DisconnectNamedPipe(m_hServer))
	{
		PrintErrorWithFunctionName(L"DisconnectNamedPipe");
		return false;
	}

	if (!ConnectPipe())
	{
		xlog(LOG_ERROR, L"error in ConnectPipe()\n");
		return false;
	}

	return true;
}


/*++

Routine Description:


Arguments:


Return Value:

	Returns TRUE on success, FALSE otherwise
--*/
BOOL PipeTransportManager::ConnectPipe()
{
	dbg(L"Connecting NamedPipe '%s'...\n", CFB_PIPE_NAME);

	BOOL fSuccess = ::ConnectNamedPipe(m_hServer, &m_oOverlap);
	if (fSuccess)
	{
		PrintErrorWithFunctionName(L"ConnectNamedPipe");
		::DisconnectNamedPipe(m_hServer);
		return FALSE;
	}

	DWORD gle = ::GetLastError();

	switch (gle)
	{
	case ERROR_IO_PENDING:
		m_fPendingIo = TRUE;
		m_dwServerState = ServerState::Connecting;
		break;

	case ERROR_PIPE_CONNECTED:
		m_dwServerState = ServerState::ReadyToReadFromClient;
		::SetEvent(m_oOverlap.hEvent);
		break;

	default:
		m_dwServerState = ServerState::Disconnected;
		xlog(LOG_ERROR, L"ConnectNamedPipe failed with %d.\n", gle);
		fSuccess = false;
		break;
	}

	return TRUE;
}


/*++

Synchronous send for named pipe

--*/
BOOL PipeTransportManager::SendSynchronous(_In_ const std::vector<byte>& data)
{
	if (data.size() >= MAX_ACCEPTABLE_MESSAGE_SIZE)
		return false;

	DWORD dwDataLength = (DWORD)data.size(), dwNbByteWritten;
	BOOL fSuccess = ::WriteFile(m_hServer, data.data(), dwDataLength, &dwNbByteWritten, NULL);
	if (!fSuccess || dwDataLength != dwNbByteWritten)
		return false;

	return true;
}


/*++

Synchronous receive for named pipe

--*/
std::vector<byte> PipeTransportManager::ReceiveSynchronous()
{
	auto buf = std::make_unique<byte[]>(MAX_MESSAGE_SIZE);
	RtlZeroMemory(buf.get(), MAX_MESSAGE_SIZE);

	DWORD dwRequestSize;
	BOOL fSuccess = ::ReadFile(m_hServer, buf.get(), MAX_ACCEPTABLE_MESSAGE_SIZE, &dwRequestSize, NULL);
	if (!fSuccess)
		RAISE_GENERIC_EXCEPTION("ReceiveSynchronous");

	std::vector<byte> res;
	for (DWORD i = 0; i < dwRequestSize; i++) res.push_back(buf[i]);
	return res;
}


/*++

Routine Description:

	The MainLoop function for the NamedPipe connector.

Arguments:

	Sess - the current session


Return Value:

	Returns 0 the thread execution went successfully, the value from GetLastError() otherwise.

--*/
DWORD PipeTransportManager::RunForever(_In_ Session& Sess)
{
	DWORD dwIndexObject, cbRet;
	DWORD retcode = ERROR_SUCCESS;
	HANDLE hResEvent = Sess.ResponseTasks.m_hPushEvent;
	BOOL fSuccess;

	const HANDLE Handles[4] = {
		Sess.m_hTerminationEvent,
		m_oOverlap.hEvent,
		hResEvent,
		m_hServer
	};


	while (Sess.IsRunning())
	{
		//
		// Wait for the pipe to be written to, or a termination notification event
		//

		dwIndexObject = ::WaitForMultipleObjects(_countof(Handles), Handles, FALSE, INFINITE) - WAIT_OBJECT_0;

		if (dwIndexObject < 0 || dwIndexObject >= _countof(Handles))
		{
			PrintErrorWithFunctionName(L"WaitForMultipleObjects()");
			xlog(LOG_CRITICAL, L"WaitForMultipleObjects(FrontEnd) has failed, cannot proceed...\n");
			Sess.Stop();
			continue;
		}


		//
		// if we received a termination event, stop everything
		//
		if (dwIndexObject == 0)
		{
#ifdef _DEBUG
			xlog(LOG_DEBUG, L"received termination Event\n");
#endif // _DEBUG
			Sess.Stop();
			continue;
		}


		//
		// otherwise, start by checking for pending IOs and update the state if needed
		//

		if (m_dwServerState == ServerState::Connecting)
		{
			LPOVERLAPPED ov = &m_oOverlap;
			fSuccess = ::GetOverlappedResult(m_hServer, ov, &cbRet, FALSE);

			if (!fSuccess)
			{
				//
				// assume the connection has closed
				//
				DisconnectAndReconnect();
				continue;
			}

			dbg(L"new pipe connection\n");
			m_dwServerState = ServerState::ReadyToReadFromClient;
		}


		//
		// process the state itself
		//
		if (m_dwServerState == ServerState::ReadyToReadFromClient)
		{
			try
			{
				auto task = Sess.FrontEndServer.ProcessNextRequest();

				switch (task.Type())
				{
				case TaskType::GetInterceptedIrps:	continue;
				case TaskType::EnumerateDrivers:	continue;
				case TaskType::ReplayIrp:			continue;
				}
			}
			catch (BrokenPipeException&)
			{
				xlog(LOG_WARNING, L"Broken pipe detected...\n");
				DisconnectAndReconnect();
				continue;
			}
			catch (BaseException & e)
			{
				xlog(LOG_ERROR, L"An exception occured while processing incoming message:\n%S\n", e.what());
				DisconnectAndReconnect();
				continue;
			}
		}
		else if (m_dwServerState == ServerState::ReadyToReadFromServer)
		{
			try
			{
				if (!Sess.FrontEndServer.ForwardReply())
					RAISE_GENERIC_EXCEPTION("ForwardReply");

				// change the state
				m_dwServerState = ServerState::ReadyToReadFromClient;
			}
			catch (BrokenPipeException&)
			{
				xlog(LOG_WARNING, L"Broken pipe detected...\n");
				DisconnectAndReconnect();
				continue;
			}
			catch (BaseException & e)
			{
				xlog(LOG_ERROR, L"An exception occured while processing incoming message:\n%S\n", e.what());
				continue;
			}
		}
		else
		{
			xlog(LOG_WARNING, L"Unexpected state (state=%d, fd=%d), invalid?\n", m_dwServerState, dwIndexObject);
			DisconnectAndReconnect();
		}
	}

	dbg(L"terminating thread TID=%d\n", ::GetThreadId(::GetCurrentThread()));
	return ERROR_SUCCESS;
}