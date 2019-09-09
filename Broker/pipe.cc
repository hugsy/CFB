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

This routine handles the communication with the front-end of CFB (for now, the only one implemented
is the GUI).

Once a message from the frontend is received, it is parsed and pushed as an incoming Task, and notify 
the BackEnd driver thread, then wait for an event from that same thread, notifying a response. Once the 
response is popped from the outgoing Task list, the data is sent back to the frontend.


Arguments:

	None


Return Value:
	Returns 0 the thread execution went successfully.

--*/
static DWORD FrontendConnectionHandlingThread(_In_ LPVOID /* lpParameter */)
{
	while (TRUE)
	{
		Sleep(10 * 1000);
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
BOOL StartGuiThread(_Out_ PHANDLE lpThread)
{
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(
		NULL,
		0,
		FrontendConnectionHandlingThread,
		NULL,
		0,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintErrorWithFunctionName(L"CreateThread(Gui)");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, "CreateThread(Gui) started as TID=%d\n", dwThreadId);
#endif

	*lpThread = hThread;

	return TRUE;
}