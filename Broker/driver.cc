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
	xlog(LOG_DEBUG, L"ShareHandleWithDriver() returned: %s, dwNbBytesReturned: %d\n", bRes ? L"TRUE" : L"FALSE", dwNbBytesReturned);
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
DWORD FetchNextIrpFromDevice(_In_ HANDLE hDevice, _In_ Session& Session)
{
	//
	// Probe the size of the buffer
	//
	BOOL bRes = FALSE;
	DWORD dwNbBytesReturned;
	byte* lpBuffer = nullptr;
	DWORD dwBufferSize = 0;

	do
	{
		bRes = ::ReadFile(
			hDevice,
			lpBuffer,
			dwBufferSize,
			&dwNbBytesReturned,
			NULL
		);

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
			xlog(LOG_ERROR, L"Unexpected error code: %x\n", dwErrCode);
			if (lpBuffer)
				delete[] lpBuffer;

			return dwErrCode;
		}

		//
		// Adjust the buffer and size, retry
		//
		if (lpBuffer)
			delete[] lpBuffer;

		lpBuffer = new byte[dwNbBytesReturned];
		dwBufferSize = dwNbBytesReturned;

#ifdef _DEBUG
		xlog(LOG_DEBUG, L"adjusted buffer size to %d\n", dwBufferSize);
#endif // _DEBUG

	} 
	while (bRes == FALSE);



	PINTERCEPTED_IRP_HEADER pIrp = (PINTERCEPTED_IRP_HEADER)lpBuffer;

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"New IRP received:\n");
	xlog(LOG_DEBUG, L"\t- timestamp:%llx\n", pIrp->TimeStamp);
	xlog(LOG_DEBUG, L"\t- IRQ level:%x\n", pIrp->Irql);
	xlog(LOG_DEBUG, L"\t- Major type:%x\n", pIrp->Type);
	xlog(LOG_DEBUG, L"\t- IoctlCode:%x\n", pIrp->IoctlCode);
	xlog(LOG_DEBUG, L"\t- PID=%d / TID=%d\n", pIrp->Pid, pIrp->Tid);
	xlog(LOG_DEBUG, L"\t- InputBufferLength=%d / OutputBufferLength=%d\n", pIrp->InputBufferLength, pIrp->OutputBufferLength);
	xlog(LOG_DEBUG, L"\t- DriverName:%s\n", pIrp->DriverName);
	xlog(LOG_DEBUG, L"\t- DeviceName:%s\n", pIrp->DeviceName);
#endif // _DEBUG


	//
	// todo: finish 
	// must be pushed to a local queue
	//


	delete[] lpBuffer;

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
		Sess.m_hTerminationEvent , 
		hIrpDataEvent.get(),
		Sess.RequestTasks.GetPushEventHandle(),
	};


	while ( Sess.IsRunning() )
	{
		//
		// Wait for a push event or a termination notification event
		//
		DWORD dwWaitResult = WaitForMultipleObjects(
			3,
			Handles,
			FALSE,
			INFINITE
		);

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			// Termination Event
			Sess.Stop();
			continue;

		case WAIT_OBJECT_0 + 1:
			// new IRP data Event
			dwRes = FetchNextIrpFromDevice( hDevice.get(), Sess );
			if (dwRes)
			{
				xlog(LOG_ERROR, L"FetchNextIrpFromDevice() failed with status=%x\n", dwRes);
			}
			continue;

		default:
			// new request
			break;
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

#ifdef _DEBUG
			xlog(LOG_DEBUG, L"DeviceIoControl(%x) returned: %d\n", in_task.IoctlCode(), bRes);
#endif // _DEBUG

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
		// flag task as Completed, the task can finally be deleted
		//
		in_task.SetState(TaskState::Completed);

		//delete& in_task;


		//
		// Prepare the response task
		//
		Task out_task(TaskType::IoctlResponse, lpOutputBuffer, dwOutputBufferSize, ::GetLastError());
		delete[] lpOutputBuffer;


		//
		// push to response task list
		//
		Sess.ResponseTasks.push(out_task);

	}


	return ERROR_SUCCESS;
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


	Session* Sess = reinterpret_cast<Session*>(lpParameter);
	Sess->m_hBackendThreadHandle = hThread;

	return TRUE;
}