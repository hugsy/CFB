#include "driver.h"


/*++

Routine Description:

Simple function to create and share the event associated with the userland notification 
that new data has been captured by the driver (and can be read).


Arguments:

	hDevice - a handle to the IrpDumper device

	
Return Value:

	Returns a handle to the event shared with the driver on success, INVALID_HANDLE_VALUE otherwise

--*/
static inline HANDLE ShareHandleWithDriver(_In_ HANDLE hDevice)
{
	DWORD dwNbBytesReturned;
	

	HANDLE hDataEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (!hDataEvent)
		return INVALID_HANDLE_VALUE;



	BOOL bRes = ::DeviceIoControl(
		hDevice,
		IOCTL_SetEventPointer,
		(byte*)&hDataEvent,
		sizeof(HANDLE),
		NULL,
		0,
		&dwNbBytesReturned,
		NULL
	);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"ShareHandleWithDriver() returned: %s, dwNbBytesReturned: %d\n", bRes ? L"TRUE" : L"FALSE", dwNbBytesReturned);
#endif

	if (bRes)
		return hDataEvent;

	return INVALID_HANDLE_VALUE;
}



/*++

Routine Description:

Reads the new tasks received by the FrontEnd thread, and bounces thoses requests to the backend (i.e. the driver) 
via the IOCTL codes (which can be found in Driver\Header Files\IoctlCodes.h).

The driver must acknowledge the request by sending a response (even if the result content is asynchronous).


Arguments:

	pEvent - 


Return Value:
	
	Returns 0 on success, GetLastError() otherwise

--*/
static DWORD BackendConnectionHandlingThread(_In_ LPVOID lpParameter)
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Getting a handle to the device object\n");
#endif

	Session* Sess = reinterpret_cast<Session*>(lpParameter);


	//
	// Get a handle to the device
	//
	wil::unique_handle hDevice(
		::CreateFile(
			CFB_USER_DEVICE_NAME,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		)
	);

	if (!hDevice)
	{
		PrintErrorWithFunctionName(L"CreateFile(g_hDevice");
		return ::GetLastError();
	}


	//
	// Create an event for the driver to notify the broker new data has arrived
	//
	wil::unique_handle hIrpDataEvent(
		ShareHandleWithDriver(hDevice.get())
	);

	if (!hIrpDataEvent)
	{
		PrintErrorWithFunctionName(L"ShareHandleWithDriver() failed");
		return ::GetLastError();
	}


	//
	// todo: finish by adding the routine for reading IRP from the driver and storing them locally
	//

	const HANDLE Handles[2] = { 
		Sess->hTerminationEvent , 
		Sess->RequestTasks.GetPushEventHandle()
	};


	while ( Sess->IsRunning() )
	{
		//
		// Wait for a push event or a termination notification event
		//
		DWORD dwWaitResult = WaitForMultipleObjects(
			2,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			Sess->Stop();
			continue;

		default:
			break;
		}

		//
		// pop() from request task list. Although the pop is blocking, there should always be
		// some data available (because of the Event)
		//

		auto in_task = Sess->RequestTasks.pop();
		

		//
		// send the DeviceIoControl
		//
#ifdef _DEBUG
		xlog(LOG_DEBUG, L"Type: %s, Length: %d\n", in_task.Type(), in_task.Length());
#endif
		
		byte* lpOutputBuffer = nullptr;
		DWORD dwOutputBufferSize = 0;
		DWORD dwNbBytesReturned = 0;
		BOOL bRes;

		while (TRUE)
		{
			bRes = ::DeviceIoControl(
				hDevice.get(),
				in_task.IoctlCode(),
				in_task.Data(),
				in_task.Length(),
				lpOutputBuffer,
				dwOutputBufferSize,
				&dwNbBytesReturned,
				NULL
			);

			//
			// If the ioctl was ok, we exit
			//
			if (bRes)
				break;

			DWORD dwErrCode = ::GetLastError();

			//
			// If the buffer was too small, retry with the appropriate size
			//
			if (dwErrCode == ERROR_INSUFFICIENT_BUFFER)
			{
				dwOutputBufferSize = dwNbBytesReturned;
				if (lpOutputBuffer)
					delete[] lpOutputBuffer;

				lpOutputBuffer = new byte[dwOutputBufferSize];
				continue;
			}

			break;
		}
		

		//
		// flag task as Completed
		//
		in_task.SetState(TaskState::Completed);

		delete& in_task;


		//
		// Prepare the response task
		//
		Task out_task(TaskType::IoctlResponse, lpOutputBuffer, dwOutputBufferSize, ::GetLastError());
		delete[] lpOutputBuffer;


		//
		// push to response task list
		//
		Sess->ResponseTasks.push(out_task);

	}


	return 0;
}



/*++

Routine Description:

Start the thread responsible for the communcation between the broker and the backend (i.e.
the driver).


Arguments:

	lpThread -  a pointer to the handle of the created


Return Value:

	Returns TRUE if the thread was successfully created; FALSE in any other case

--*/
_Success_(return)
BOOL StartBackendManagerThread(_In_ PVOID lpParameter)
{
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(
		NULL,
		0,
		BackendConnectionHandlingThread,
		lpParameter,
		0,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintError(L"CreateThread(Driver)");
		return FALSE;
	}

	

#ifdef _DEBUG
	xlog(LOG_DEBUG, "CreateThread(Driver) started as TID=%d\n", dwThreadId);
#endif

	Session* Sess = reinterpret_cast<Session*>(lpParameter);
	Sess->hBackendThreadHandle = hThread;

	return TRUE;
}