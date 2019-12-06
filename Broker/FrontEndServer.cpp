#include "FrontEndServer.h"


#include <iostream>
#include <locale>
#include <codecvt>

#include "taskmanager.h"
#include "Session.h"
#include "task.h"


#include "json.hpp"
using json = nlohmann::json;





/*++
Routine Description:

Creates the FrontEndServer object.


Arguments:

	None


Return Value:
	May throw an exception if the the allocation failed.

--*/
FrontEndServer::FrontEndServer(Session& Session) noexcept(false)
	: m_Session(Session)
{
	if (!m_Transport.Initialize())
		RAISE_GENERIC_EXCEPTION("m_Transport.Initialize() failed");
}


/*++
Routine Description:

Destroys the FrontEndServer object.


Arguments:

	None


Return Value:
	May throw an exception if the the deallocation failed.

--*/
FrontEndServer::~FrontEndServer() noexcept(false)
{
	if (!m_Transport.Terminate())
		RAISE_GENERIC_EXCEPTION("m_Transport.Terminate() failed");
}


Task FrontEndServer::ProcessNextRequest()
{
	//
	// read the json message
	//
	auto RequestBufferRaw = m_Session.FrontEndServer.Receive();

#ifdef _DEBUG
	SIZE_T dwRequestSize = RequestBufferRaw.size();
	dbg(L"new pipe message (len=%lu)\n", dwRequestSize);
	//hexdump(RequestBufferRaw.data(), dwRequestSize);
#endif // _DEBUG

	const std::string request_str(RequestBufferRaw.begin(), RequestBufferRaw.end());
	auto json_request = json::parse(request_str);
	const TaskType type = static_cast<TaskType>(json_request["body"]["type"]);
	DWORD dwDataLength = json_request["body"]["data_length"];
	auto data = json_request["body"]["data"].get<std::string>();
	auto lpData = Utils::base64_decode(data);

	assert(lpData.size() == dwDataLength);

	//
	// build a Task object from the next message read from the pipe
	//
	Task task(type, lpData.data(), dwDataLength, -1);


	dbg(L"new request task (id=%d, type='%s', length=%d)\n", task.Id(), task.TypeAsString(), task.Length());

	switch (task.Type())
	{
	case TaskType::GetInterceptedIrps:
		SendInterceptedIrps();
		break;

	case TaskType::EnumerateDrivers:
		SendDriverList();
		break;

	case TaskType::ReplayIrp:
		//
		// Replay the IRP
		//
		// TODO
	default:
		// push the task to request task list
		m_Session.RequestTasks.push(task);
	}

	return task;
}


BOOL FrontEndServer::ForwardReply()
{

	//
	// pop the response task and build the json message
	//
	auto task = m_Session.ResponseTasks.pop();

	dbg(L"new response task (id=%d, type='%s', length=%d, gle=%d)\n", task.Id(), task.TypeAsString(), task.Length(), task.ErrCode());

	json json_response = {
		{"header", {
			{"success", task.ErrCode() == ERROR_SUCCESS},
			{"gle", task.ErrCode()},
		}
	} };

	json_response["body"]["data_length"] = task.Length();
	if (task.Length() > 0)
		json_response["body"]["data"] = Utils::base64_encode(task.Data(), task.Length());

	const std::string& str = json_response.dump();
	const std::vector<byte> raw(str.begin(), str.end());
	if (!m_Session.FrontEndServer.Send(raw))
	{
		PrintErrorWithFunctionName(L"SendSynchronous(Ioctl)");
	}
	else
	{
		task.SetState(TaskState::Completed);
	}

	return true;
}



/*++

Routine Description:


Arguments:
	
	Session -


Return Value:

	Returns 0 on success, -1 on failure.

--*/
DWORD FrontEndServer::SendInterceptedIrps()
{
	json j = {
		{"header", {
			{"success", true},
			{"gle", ERROR_SUCCESS}
		}
	} };

	//
	// Make sure no element are being added concurrently
	//
	std::unique_lock<std::mutex> mlock(m_Session.m_IrpMutex);
	size_t i = 0;

	j["body"]["entries"] = json::array();

	while(!m_Session.m_IrpQueue.empty())
	{
		//
		// pop an IRP
		//
		Irp irp(m_Session.m_IrpQueue.front());
		m_Session.m_IrpQueue.pop();

		//
		// format a new JSON entry
		//
		j["body"]["entries"].push_back(irp.ToJson());

		i++;
	}

	mlock.unlock();

	j["body"]["nb_entries"] = i;


	//
	// Write the data back
	//

	const std::string& str = j.dump();
	const std::vector<byte> raw(str.begin(), str.end());

	if (!m_Session.FrontEndServer.Send(raw))
	{
		PrintErrorWithFunctionName(L"SendSynchronous(Irps)");
		return ERROR_INVALID_DATA;
	}

	return ERROR_SUCCESS;
}



/*++

Routine Description:

	Sends the list of drivers

Arguments:

	Session -


Return Value:

	Returns 0 on success, -1 on failure.

--*/
DWORD FrontEndServer::SendDriverList()
{
	int i=0;
	
	json j = {
		{"header", {
			{"success", true},
			{"gle", ERROR_SUCCESS}
		}
	}};


	j["body"]["drivers"] = json::array();
	
	// wstring -> string converter
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wide_converter;
	std::vector<std::wstring> roots = { L"\\Driver" , L"\\FileSystem" };
	for (auto root : roots)
	{
		for (auto driver : Utils::EnumerateObjectDirectory(root))
		{
			std::wstring driver_abspath = root + std::wstring(L"\\") + driver.first;
			std::string driver_name = wide_converter.to_bytes(driver_abspath);
			j["body"]["drivers"].push_back(driver_name);
			i++;
		}
	}


	const std::string& str = j.dump();
	const std::vector<byte> raw(str.begin(), str.end());

	if (!m_Session.FrontEndServer.Send(raw))
	{
		PrintErrorWithFunctionName(L"SendSynchronous(Drivers)");
		return ERROR_INVALID_DATA;
	}

	return ERROR_SUCCESS;
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
static DWORD ServerThreadRoutine(_In_ LPVOID lpParameter)
{
	Session& Sess = *(reinterpret_cast<Session*>(lpParameter));
	return Sess.FrontEndServer.RunForever();
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
BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter)
{
	DWORD dwThreadId;

	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		ServerThreadRoutine,
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

	Session& Sess = *(reinterpret_cast<Session*>(lpParameter));
	Sess.m_hFrontendThread = hThread;

	return TRUE;
}

