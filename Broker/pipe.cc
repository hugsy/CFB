#include "pipe.h"


/*++

Create the named pipe responsible for the communication with the GUI.

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
		PrintError(L"CreateNamedPipe()");
		return FALSE;
	}

	return TRUE;
}


/*++

Flush all the data and close the pipe.

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
		PrintError(L"DisconnectNamedPipe()");
		return FALSE;
	}

	return TRUE;
}


/*++

--*/
static DWORD GuiThread(_In_ LPVOID lpParameter)
{
	while (TRUE)
	{
		Sleep(10 * 1000);
	}

	return 0;
}



/*++

--*/
_Success_(return) 
BOOL StartGuiThread(_Out_ PHANDLE lpThread)
{
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(
		NULL,
		0,
		GuiThread,
		NULL,
		0,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintError(L"CreateThread(Gui)");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, "CreateThread(Gui) started as TID=%d\n", dwThreadId);
#endif

	*lpThread = hThread;

	return TRUE;
}