#include "pipe.h"



/*++
Routine Description:

Create the named pipe responsible for the communication with the GUI.


Arguments:

	None


Return Value:
	Returns TRUE upon successful creation of the pipe, FALSE if any error occured.

--*/
BOOL CreateServerPipe()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Creating named pipe '%s'...\n", CFB_PIPE_NAME);
#endif

	g_hServerPipe = CreateNamedPipe(
		CFB_PIPE_NAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
		CFB_PIPE_MAXCLIENTS,
		CFB_PIPE_INBUFLEN,
		CFB_PIPE_OUTBUFLEN,
		0,
		NULL
	);

	if (g_hServerPipe == INVALID_HANDLE_VALUE)
	{
		PrintErrorWithFunctionName(L"CreateNamedPipe()");
		return FALSE;
	}

	return TRUE;
}


/*++

Routine Description:

Flush all the data and close the pipe.


Arguments:

	None


Return Value:
	Returns TRUE upon successful termination of the pipe, FALSE if any error occured.

--*/
BOOL CloseServerPipe()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Closing named pipe '%s'...\n", CFB_PIPE_NAME);
#endif

	//
	// Wait until all data was consumed
	//
	FlushFileBuffers(g_hServerPipe);

	//
	// Then close down the named pipe
	//
	if (!DisconnectNamedPipe(g_hServerPipe))
	{
		PrintErrorWithFunctionName(L"DisconnectNamedPipe()");
		return FALSE;
	}

	return TRUE;
}


/*++

Routine Description:

Reads and validate a message from the named pipe (from the frontend). Each message format must follow TLV type of format as follow:

- Type as uint32_t
- Length as uint32_t 
- Value as uint8_t[Length]


Arguments:

	hPipe - the handle to read data from


Return Value:

	a correctly formatted Task object on success, a C++ exception otherwise

--*/
Task ReadPipeMessage(HANDLE hPipe)
{
	BOOL bSuccess = FALSE;
	DWORD dwNbByteRead;

	//
	// Read Type / Length
	//
	uint32_t tl[2] = { 0 };

	bSuccess = ReadFile(
		hPipe,
		tl,
		sizeof(tl),
		&dwNbByteRead,
		NULL
	);

	if (!bSuccess || dwNbByteRead != sizeof(tl))
		throw std::runtime_error("ReadFile() failed:");

	if(tl[0] >= TaskType::TaskTypeMax)
		throw std::runtime_error("Message type is invalid");

	TaskType type = static_cast<TaskType>(tl[0]);
	uint32_t datalen = tl[1];


	//
	// then allocate, and read the data
	//
	byte* data = new byte[datalen];
	if(data == nullptr)
		throw std::runtime_error("allocate failed");

	bSuccess = ReadFile(
		hPipe,
		data,
		datalen,
		&dwNbByteRead,
		NULL
	);

	if (!bSuccess || dwNbByteRead != datalen)
		throw std::runtime_error("ReadFile() failed:");

	auto task = Task(type, data, datalen);
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
static byte* PreparePipeMessageOut(_In_ Task task )
{
	byte* msg;

	//
	// write response to pipe
	//
	uint32_t msglen = 2 * sizeof(uint32_t) + task.Length();

	if (msglen >= 2 * sizeof(uint32_t))
	{
		msg = new byte[msglen];

		uint32_t* tl = reinterpret_cast<uint32_t*>(msg);
		tl[0] = task.Type();
		tl[1] = task.Length();
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
	Returns 0 the thread execution went successfully.

--*/
static DWORD FrontendConnectionHandlingThreadIn(_In_ LPVOID lpParameter)
{
	HANDLE Handles[2] = { 0 };
	Handles[0] = *((PHANDLE)lpParameter);;
	DWORD dwNumberOfBytesWritten, dwWaitResult;


	while (g_bIsRunning)
	{
		//
		// Wait for the pipe to be written to, or a termination notification event
		//
		
		Handles[1] = g_hServerPipe;

		dwWaitResult = WaitForMultipleObjects(
			2,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			g_bIsRunning = FALSE;
			continue;

		default:
			break;
		}

		try
		{
			//
			// get next message from pipe and parse it as a task
			//
			auto task = ReadPipeMessage(g_hServerPipe);


			//
			// push the task to request task list (and set the push event)
			//
			g_RequestManager.push(task);
			task.SetState(Queued);

		}
		catch (std::exception e)
		{
			xlog(LOG_WARNING, L"Invalid message format, discarding: %S\n", e.what());
			continue;
		}



		//
		// Wait for either the response or a termination event
		//
		Handles[1] = g_ResponseManager.hPushEvent;

		dwWaitResult = WaitForMultipleObjects(
			2,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			g_bIsRunning = FALSE;
			continue;

		default:
			break;
		}


		try
		{

			//
			// blocking-pop on response task list
			//
			auto task = g_ResponseManager.pop();

			byte* msg = PreparePipeMessageOut(task);
			DWORD msglen = task.Length() + 2 * sizeof(uint32_t);

			BOOL bRes = WriteFile(
				g_hServerPipe,
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
			delete& task;
			delete[] msg;
		}
		catch(std::exception e)
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

	lpThread - a pointer to the handle of the created


Return Value:
	Returns TRUE upon successful creation of the thread, FALSE if any error occured.

--*/
_Success_(return) 
BOOL StartFrontendManagerThread(_In_ PHANDLE pEvent, _Out_ PHANDLE lpThread)
{
	DWORD dwThreadInId;

	HANDLE hThreadIn = CreateThread(
		NULL,
		0,
		FrontendConnectionHandlingThreadIn,
		pEvent,
		0,
		&dwThreadInId
	);

	if (!hThreadIn)
	{
		PrintErrorWithFunctionName(L"CreateThread(hThreadPipeIn)");
		return FALSE;
	}
	

#ifdef _DEBUG
	xlog(LOG_DEBUG, "Created frontend thread as TID=%d\n", dwThreadInId);
#endif

	*lpThread = hThreadIn;

	return TRUE;
}