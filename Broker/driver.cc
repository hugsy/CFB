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
	

	HANDLE hDataEvent = ::CreateEvent(
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
	xlog(LOG_DEBUG, L"ShareHandleWithDriver() returned: %s\n", bRes ? L"TRUE" : L"FALSE");
#endif

	if (bRes)
		return hDataEvent;

	return INVALID_HANDLE_VALUE;
}


/*++

Routine Description:

Fetch one IRP (metadata + data) temporarily stored in the driver.


Arguments:

	hDevice -

	lpSession -


Return Value:

	Returns ERROR_SUCCESS on success, GetLastError() otherwise

--*/
DWORD FetchNextIrpFromDevice(_In_ HANDLE hDevice, _In_ HANDLE hEvent, _In_ Session& Session)
{
	//
	// Probe the size of the buffer
	//
	BOOL bRes = FALSE;
	DWORD lpNumberOfBytesRead;
	DWORD dwBufferSize = 0;
	byte* lpBuffer = NULL;

	do
	{
		bRes = ::ReadFile(
			hDevice,
			lpBuffer,
			dwBufferSize,
			&lpNumberOfBytesRead,
			NULL
		);

		xlog(LOG_DEBUG, L"ReadFile(hDevice=%p, lpBuffer=%p, dwBufferSize=%d) -> %d\n", hDevice, lpBuffer, dwBufferSize, bRes);

		if (bRes == TRUE)
			//
			// lpBuffer was correctly filled with the IRP, we can stop probing
			//
			break;


		//
		// Otherwise, check the error status
		//
		DWORD dwErrCode = ::GetLastError();
		
		if (dwErrCode != ERROR_INSUFFICIENT_BUFFER)
		{
			PrintErrorWithFunctionName(L"ReadFile(hDevice)");
			if (lpBuffer)
				delete[] lpBuffer;

			return dwErrCode;
		}


		//
		// Adjust the buffer and size, retry
		//
		if (lpBuffer)
			delete[] lpBuffer;

		lpBuffer = new byte[lpNumberOfBytesRead];
		dwBufferSize = lpNumberOfBytesRead;

#ifdef _DEBUG
		xlog(LOG_DEBUG, L"adjusted buffer size to %d\n", dwBufferSize);
#endif // _DEBUG

	} 
	while (bRes == FALSE);



	PINTERCEPTED_IRP_HEADER pIrpHeader = (PINTERCEPTED_IRP_HEADER)lpBuffer;
	PINTERCEPTED_IRP_BODY pIrpBody = (PINTERCEPTED_IRP_BODY)(lpBuffer + sizeof(INTERCEPTED_IRP_HEADER));

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"New IRP received:\n");
	xlog(LOG_DEBUG, L"\t- timestamp:%llx\n", pIrpHeader->TimeStamp);
	xlog(LOG_DEBUG, L"\t- IRQ level:%x\n", pIrpHeader->Irql);
	xlog(LOG_DEBUG, L"\t- Major type:%x\n", pIrpHeader->Type);
	xlog(LOG_DEBUG, L"\t- IoctlCode:%x\n", pIrpHeader->IoctlCode);
	xlog(LOG_DEBUG, L"\t- PID=%d / TID=%d\n", pIrpHeader->Pid, pIrpHeader->Tid);
	xlog(LOG_DEBUG, L"\t- InputBufferLength=%d / OutputBufferLength=%d\n", pIrpHeader->InputBufferLength, pIrpHeader->OutputBufferLength);
	xlog(LOG_DEBUG, L"\t- DriverName:%s\n", pIrpHeader->DriverName);
	xlog(LOG_DEBUG, L"\t- DeviceName:%s\n", pIrpHeader->DeviceName);
#endif // _DEBUG


	//
	// pushing new IRP to the session queue\n");
	//

	Irp irp(pIrpHeader, pIrpBody);

	std::unique_lock<std::mutex> mlock(Session.m_IrpMutex);
	Session.m_IrpQueue.push(irp);
	mlock.unlock();

	//
	// Reset the event
	//
	ResetEvent(hEvent);

	return ERROR_SUCCESS;
}


/*++

Routine Description:

Reads the new tasks received by the FrontEnd thread, and bounces thoses requests to the backend (i.e. the driver) 
via the IOCTL codes (which can be found in "Driver\Header Files\IoctlCodes.h" - imported by common.h).

The driver must acknowledge the request by sending a response (even if the result content is asynchronous).


Arguments:

	lpParameter - 


Return Value:
	
	Returns 0 on success

--*/
DWORD BackendConnectionHandlingThread(_In_ LPVOID lpParameter)
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Getting a handle to the device object\n");
#endif // _DEBUG


	Session& Sess = *(reinterpret_cast<Session*>(lpParameter));
	DWORD dwRes;
	DWORD retcode = ERROR_SUCCESS;


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
		PrintErrorWithFunctionName(L"CreateFile(hDeviceObject)");
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
		PrintErrorWithFunctionName(L"ShareHandleWithDriver()");
		return ERROR_INVALID_HANDLE;
	}


	//
	// todo: finish by adding the routine for reading IRP from the driver and storing them locally
	//

	const HANDLE Handles[3] = { 
		Sess.m_hTerminationEvent, 
		hIrpDataEvent.get(),
		Sess.RequestTasks.m_hPushEvent
	};


	while ( Sess.IsRunning() )
	{
		//
		// Wait for a push event or a termination notification event
		//
		DWORD dwWaitResult = ::WaitForMultipleObjects(_countof(Handles), Handles, FALSE, INFINITE);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
#ifdef _DEBUG
			xlog(LOG_DEBUG, L"received termination Event\n");
#endif // _DEBUG

			// Termination Event
			Sess.Stop();
			continue;

		case WAIT_OBJECT_0 + 1:
#ifdef _DEBUG
			xlog(LOG_DEBUG, L"new IRP data Event\n");
#endif // _DEBUG

			dwRes = FetchNextIrpFromDevice( hDevice.get(), hIrpDataEvent.get(), Sess );
			continue;

		case WAIT_OBJECT_0 + 2:
			// new request from front end
#ifdef _DEBUG
			xlog(LOG_DEBUG, L"new request from FrontEnd\n");
#endif // _DEBUG
			
			break;

		default:
			PrintErrorWithFunctionName(L"WaitForMultipleObjects()");
			xlog(LOG_CRITICAL, "WaitForMultipleObjects(BackEnd) has failed, cannot proceed...\n");
			Sess.Stop();
			continue;
		}

		//
		// pop() from request task list. Although the pop is blocking, there should always be
		// some data available (because of the Event)
		//

		auto in_task = Sess.RequestTasks.pop();
		

		//
		// send the DeviceIoControl
		//
#ifdef _DEBUG
		xlog(LOG_DEBUG, L"Sending to device Task=%d (Type: %s, Length: %d)\n", in_task.Id(), in_task.TypeAsString(), in_task.Length());
#endif // _DEBUG

		
		byte* lpOutputBuffer = nullptr;
		DWORD dwOutputBufferSize = 0;
		DWORD dwNbBytesReturned = 0;
		BOOL bRes;
		DWORD dwErrCode;

		while (TRUE)
		{
			dwErrCode = ERROR_SUCCESS;

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

#ifdef _DEBUG
			xlog(LOG_DEBUG, L"DeviceIoControl(%x) returned: %d\n", in_task.IoctlCode(), bRes);
#endif // _DEBUG

			//
			// If the ioctl was ok, we exit
			//
			if (bRes)
				break;

			dwErrCode = ::GetLastError();


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
		// flag task as Completed, the task can finally be deleted
		//
		in_task.SetState(TaskState::Completed);


		//
		// Prepare the response task
		// A response task always starts with the response code (DWORD), then optionally the data if any
		//
		byte* lpResponseBuffer = new byte[dwOutputBufferSize+sizeof(DWORD)];
		::memcpy(lpResponseBuffer, &dwErrCode, sizeof(DWORD));
		::memcpy(lpResponseBuffer+sizeof(DWORD), lpOutputBuffer, dwOutputBufferSize);
		delete[] lpOutputBuffer;


		Task out_task(TaskType::IoctlResponse, lpResponseBuffer, dwOutputBufferSize+sizeof(DWORD));
		delete[] lpResponseBuffer;


		//
		// push to response task list
		//
		Sess.ResponseTasks.push(out_task);

	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"terminating thread TID=%d with retcode=%d\n", GetThreadId(GetCurrentThread()), retcode);
#endif // _DEBUG

	return retcode;
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

	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		BackendConnectionHandlingThread,
		lpParameter,
		CREATE_SUSPENDED,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintError(L"CreateThread(Driver)");
		return FALSE;
	}

	
#ifdef _DEBUG
	xlog(LOG_DEBUG, "CreateThread(Driver) started as TID=%d\n", dwThreadId);
#endif // _DEBUG


	Session& Sess = *(reinterpret_cast<Session*>(lpParameter));
	Sess.m_hBackendThreadHandle = hThread;

	return TRUE;
}