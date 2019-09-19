#include "FrontEndServer.h"



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
BOOL FrontEndServer::CreatePipe()
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
#ifdef _DEBUG
		xlog(LOG_DEBUG, L"Defining new SD for pipe\n");
#endif // _DEBUG
	

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

		m_hServerHandle = ::CreateNamedPipe(
			CFB_PIPE_NAME,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS,
			CFB_PIPE_MAXCLIENTS,
			CFB_PIPE_INBUFLEN,
			CFB_PIPE_OUTBUFLEN,
			0,
			&SecurityAttributes
		);

		if (m_hServerHandle == INVALID_HANDLE_VALUE)
		{
			PrintErrorWithFunctionName(L"CreateNamedPipe()");
			fSuccess = FALSE;
			break;
		}

		fSuccess = TRUE;
	} 
	while (FALSE);

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
BOOL FrontEndServer::ClosePipe()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Closing named pipe '%s'...\n", CFB_PIPE_NAME);
#endif

	BOOL fSuccess = TRUE;

	do
	{
		//
		// Wait until all data was consumed
		//
		if (!FlushFileBuffers(m_hServerHandle))
		{
			PrintErrorWithFunctionName(L"FlushFileBuffers()");
			fSuccess = FALSE;
		}

		//
		// Then close down the named pipe
		//
		if (!DisconnectNamedPipe(m_hServerHandle))
		{
			PrintErrorWithFunctionName(L"DisconnectNamedPipe()");
			fSuccess = FALSE;
		}
	} 
	while (FALSE);

	return fSuccess;
}

BOOL FrontEndServer::ListenPipe()
{
	if (!::ConnectNamedPipe(m_hServerHandle, NULL))
	{
		PrintErrorWithFunctionName(L"ConnectNamedPipe");
		return FALSE;
	}

	return TRUE;
}


HANDLE FrontEndServer::GetListeningSocketHandle()
{
	return m_hServerHandle;
}


/*++

Routine Description:

Reads and validate a message from the named pipe (from the frontend). Each message format must follow TLV type of format as follow:

- Type as uint32_t
- Length as uint32_t (the length of the field `Value`)
- Value as uint8_t[Length]

This means that each message has a TOTAL SIZE of at least 2*sizeof(uint32_t) bytes (i.e. 8 bytes), that is if Length=0


Arguments:

	hPipe - the handle to read data from


Return Value:

	a correctly formatted Task object on success, a C++ exception otherwise

--*/
Task ReadTlvMessage(_In_ HANDLE hPipe) 
{
	BOOL bSuccess = FALSE;
	DWORD dwNbByteRead;

	//
	// Read Type / Length
	//
	uint32_t tl[2] = { 0 };

	bSuccess = ::ReadFile(
		hPipe,
		tl,
		sizeof(tl),
		&dwNbByteRead,
		NULL
	);

	if (!bSuccess)
	{
		switch (::GetLastError())
		{
		case ERROR_BROKEN_PIPE:
			RAISE_EXCEPTION(BrokenPipeException, "ReadFile(1) failed");

		default:
			RAISE_GENERIC_EXCEPTION("ReadFile(1) failed");
		}
	}
		

	if (dwNbByteRead != sizeof(tl))
		RAISE_GENERIC_EXCEPTION("ReadFile(1): invalid size read");

	if (tl[0] >= TaskType::TaskTypeMax)
		RAISE_GENERIC_EXCEPTION("ReadFile(1): Message type is invalid");

	TaskType type = static_cast<TaskType>(tl[0]);
	uint32_t datalen = tl[1];


	//
	// then allocate, and read the data
	//
	byte* data = new byte[datalen];
	if (data == nullptr)
		throw std::runtime_error("allocate failed");

	if (datalen)
	{
		bSuccess = ::ReadFile(
			hPipe,
			data,
			datalen,
			&dwNbByteRead,
			NULL
		);

		if (!bSuccess)
			throw std::runtime_error("ReadFile(2) failed");

		if (dwNbByteRead != datalen)
			throw std::runtime_error("ReadFile(2): invalid size read");
	}

	Task task(type, data, datalen);
	delete[] data;

	return task;
}


/*++

Routine Description:


Arguments:

	task


Return Value:

	Returns 0 if successful.

--*/
byte* PrepareTlvMessageFromTask(_In_ Task& task)
{
	byte* msg = nullptr;

	//
	// write response to pipe
	//
	uint32_t msglen = 2 * sizeof(uint32_t) + task.Length();
	
	if (msglen >= 2 * sizeof(uint32_t))
	{
		msg = new byte[msglen];

		// copy header
		uint32_t* tl = reinterpret_cast<uint32_t*>(msg);
		tl[0] = task.Type();
		tl[1] = task.Length();

		// copy the body
		::memcpy(msg + 2 * sizeof(uint32_t), task.Data(), task.Length());
	}
	else
	{
		throw std::runtime_error("arithmetic overflow");
	}

	return msg;
}


/*++

Routine Description:

This routine handles the communication with the front-end of CFB (for now, the only one implemented
is the GUI).

Once a message from the frontend is received, it is parsed and pushed as an incoming Task, and notify
the BackEnd driver thread, then wait for an event from that same thread, notifying a response. Once the
response is popped from the outgoing Task list, the data is sent back to the frontend.


Arguments:

	lpParameter - the thread parameter


Return Value:

	Returns 0 the thread execution went successfully, the value from GetLastError() otherwise.

--*/
DWORD FrontendConnectionHandlingThread(_In_ LPVOID lpParameter)
{
	Session& Sess = *(reinterpret_cast<Session*>(lpParameter));
	DWORD dwNumberOfBytesWritten, dwWaitResult;
	HANDLE Handles[2] = { 0 };
	Handles[0] = Sess.m_hTerminationEvent;
	HANDLE hServer = Sess.FrontEndServer.GetListeningSocketHandle();

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Pipe mode listening...\n");
#endif // _DEBUG


	if (!Sess.FrontEndServer.ListenPipe())
		return ERROR_INVALID_PARAMETER;


	while (Sess.IsRunning())
	{
		//
		// Wait for the pipe to be written to, or a termination notification event
		//

		Handles[1] = hServer;

		dwWaitResult = ::WaitForMultipleObjects(
			2,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			Sess.Stop();
			continue;

		case WAIT_OBJECT_0 + 1:
			// normal case
			break;

		default:
			PrintErrorWithFunctionName(L"WaitForMultipleObjects()");
			xlog(LOG_CRITICAL, "WaitForMultipleObjects(FrontEnd) has failed, cannot proceed...\n");
			Sess.Stop();
			continue;
		}


		try
		{
			//
			// Construct a Task object from the next message read from the pipe
			//
			auto task = ReadTlvMessage(hServer);

#ifdef _DEBUG
			xlog(LOG_DEBUG, L"new task (id=%d, type='%s')\n", task.Id(), task.TypeAsString());
#endif // _DEBUG

			//
			// push the task to request task list
			//
			Sess.RequestTasks.push(task);

		}
		catch (BrokenPipeException&)
		{
			xlog(LOG_WARNING, L"Broken pipe detected\n");
			Sess.Stop(); // todo: handle reconnect
			continue;
		}
		catch (BaseException& e)
		{
			xlog(LOG_ERROR, L"An exception occured while processing incoming message:\n%S\n", e.what());
			continue;
		}



		//
		// Wait for either the response or a termination event
		//
		Handles[1] = Sess.ResponseTasks.GetPushEventHandle();

		dwWaitResult = WaitForMultipleObjects(
			2,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			Sess.Stop();
			continue;

		case WAIT_OBJECT_0 + 1:
			// normal case
			break;

		default:
			PrintErrorWithFunctionName(L"WaitForMultipleObjects()");
			xlog(LOG_CRITICAL, "WaitForMultipleObjects(FrontEnd) has failed, cannot proceed...\n");
			Sess.Stop();
			continue;
		}


		try
		{

			//
			// blocking-pop on response task list
			//
			auto task = Sess.ResponseTasks.pop();

			byte* msg = PrepareTlvMessageFromTask(task);
			DWORD msglen = task.Length() + 2 * sizeof(uint32_t);

			BOOL bRes = ::WriteFile(
				hServer,
				msg,
				msglen,
				&dwNumberOfBytesWritten,
				NULL
			);

			if (!bRes || dwNumberOfBytesWritten != msglen)
			{
				PrintErrorWithFunctionName(L"WriteFile()");
			}


			//
			// cleanup
			//
			delete[] msg;
		}
		catch (std::exception& e)
		{
			xlog(LOG_WARNING, L"Invalid message format, discarding: %S\n", e.what());
			continue;
		}
	}

	return 0;
}



/*++

Routine Description:

This function is a simple wrapper around CreateThread() to start the thread handling the conversation
with the frontend part of the application.


Arguments:

	lpParameter - a generic pointer to the global Session


Return Value:
	Returns TRUE upon successful creation of the thread, FALSE if any error occured.

--*/
_Success_(return)
BOOL StartFrontendManagerThread(_In_ LPVOID lpParameter)
{
	DWORD dwThreadId;

	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		FrontendConnectionHandlingThread,
		lpParameter,
		CREATE_SUSPENDED,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintErrorWithFunctionName(L"CreateThread(hThreadPipeIn)");
		return FALSE;
	}


#ifdef _DEBUG
	xlog(LOG_DEBUG, "Created frontend thread as TID=%d\n", dwThreadId);
#endif

	Session* Sess = reinterpret_cast<Session*>(lpParameter);
	Sess->m_hFrontendThreadHandle = hThread;

	return TRUE;
}