#include "FrontEndServer.h"

#include <algorithm>
#include <iostream>
#include <locale>
#include <codecvt>
#include <optional>

#include "taskmanager.h"
#include "Session.h"
#include "task.h"
#include "Utils.h"


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
FrontEndServer::FrontEndServer(_In_ Session& Session) noexcept(false)
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



std::vector<Task> FrontEndServer::ProcessNextRequest()
{
	std::vector<Task> tasks;

	while (true)
	{
		//
		// read the bytes from the wire
		//
		auto RequestBufferRaw = m_Session.FrontEndServer.Receive();

		if (RequestBufferRaw.size() == 0)
			RAISE_EXCEPTION(InvalidRequestException, "Receive() should not be empty");

#ifdef _DEBUG
		SIZE_T dwRequestSize = RequestBufferRaw.size();
		dbg(L"new message from client (len=%lu)\n", dwRequestSize);
#endif // _DEBUG


		//
		// json messages can arrived fragmented, so we concat the data received to the data 
		// from potential previous read
		//
		std::move(RequestBufferRaw.begin(), RequestBufferRaw.end(), std::back_inserter(m_ReceivedBytes));

		// one message can contain multiple json, process for every one of them
		while (true)
		{
			if (auto json_request = GetNextJsonStringMessage())
			{
				auto t = ProcessJsonTask(*json_request);
				tasks.push_back(t);
			}
			else
			{
				break;
			}
		}

		//
		// if we've read everything, m_ReceivedBytes should be empty
		//
		if (m_ReceivedBytes.size() == 0)
			break;
	}

	dbg(L"got %d task(s)\n", tasks.size());

	return tasks;
}


/*++

Consume from m_ReceivedBytes until it receives a valid JSON message, or throws an exception if non available

--*/
std::optional<std::string> FrontEndServer::GetNextJsonStringMessage()
{
	unsigned int level = 0;
	size_t offset_current = 0;

	for (auto b : m_ReceivedBytes)
	{
		if (b == '{') level++;
		if (b == '}') level--;

		if (level == 0 && offset_current)
		{
			offset_current++;
			std::string jsonstr = std::string(m_ReceivedBytes.begin(), m_ReceivedBytes.begin() + offset_current);
			m_ReceivedBytes.erase(m_ReceivedBytes.begin(), m_ReceivedBytes.begin() + offset_current);
			return jsonstr;
		}

		offset_current++;
	}

	return {};
}



Task FrontEndServer::ProcessJsonTask(const std::string& json_request_as_string)
{
	// todo: catch json exception cleanly
	auto json_request = json::parse(json_request_as_string);
	const TaskType type = static_cast<TaskType>(json_request["header"]["type"]);
	DWORD dwDataLength = json_request["body"]["param_length"];
	auto data = json_request["body"]["param"].get<std::string>();
	auto lpData = Utils::base64_decode(data);

	assert(lpData.size() == dwDataLength);

	dbg(L"json request:\n%S\n", json_request.dump().c_str());


	//
	// build a Task object from the next message read from the pipe
	//
	Task task(type, lpData.data(), dwDataLength, -1, true);


	dbg(L"new request task (id=%d, type='%s', length=%d)\n", task.Id(), task.TypeAsString(), task.Length());

	switch (task.Type())
	{
	case TaskType::GetOsInfo:
		SendOsInfo();
		dbg(L"task %s done\n", task.TypeAsString());
		task.SetState(TaskState::Completed);
		break;

	case TaskType::GetInterceptedIrps:
		SendInterceptedIrps();
		task.SetState(TaskState::Completed);
		break;

	case TaskType::EnumerateDrivers:
		SendDriverList(); 
		task.SetState(TaskState::Completed);
		break;

	case TaskType::ReplayIrp:
		SendForgedIrp(json_request);
		task.SetState(TaskState::Completed);
		break;

	default:
		// push the task to request task list to be handled by the backend thread
		m_Session.RequestTasks.push(task);
	}

	return task;
}




DWORD FrontEndServer::SendReply(json& j)
{
	const std::string& str = j.dump();
	const std::vector<byte> raw(str.begin(), str.end());

	return m_Session.FrontEndServer.Send(raw)
		? ERROR_SUCCESS
		: ERROR_INVALID_DATA;
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
			{"is_success", task.ErrCode() == ERROR_SUCCESS},
			{"gle", task.ErrCode()},
		}
	} };


	//json_response["body"]["data_length"] = task.Length();

	switch (task.Type())
	{
	case TaskType::GetDriverInfo:
	{
		// data is HOOKED_DRIVER_INFO
		PHOOKED_DRIVER_INFO data = (PHOOKED_DRIVER_INFO)task.Data();
		if (task.Length() && data)
		{
			json_response["body"]["driver_info"]["driver"]["Address"] = data->DriverAddress;
			json_response["body"]["driver_info"]["driver"]["IsEnabled"] = data->Enabled;
			json_response["body"]["driver_info"]["driver"]["Name"] = Utils::WideStringToString(std::wstring(data->Name));
			json_response["body"]["driver_info"]["driver"]["NumberOfRequestIntercepted"] = data->NumberOfRequestIntercepted;
		}
		break;
	}

	case TaskType::GetNamesOfHookedDrivers:
	{
		auto driver_list_ws = std::wstring((wchar_t*)task.Data());
		auto driver_list_cs = Utils::WideStringToString(driver_list_ws);

		std::string delimiter = ";";
		size_t pos = 0;
		json_response["body"]["hooked_driver_list"]["drivers"] = json::array();
		while ((pos = driver_list_cs.find(delimiter)) != std::string::npos)
		{
			std::string drvname = driver_list_cs.substr(0, pos);
			json_response["body"]["hooked_driver_list"]["drivers"].push_back(drvname);
			driver_list_cs.erase(0, pos + delimiter.length());
		}

		break;
	}

	default:
	{
		// default = data needs no parsing, simply b64encode it
		json_response["body"]["data_length"] = task.Length();
		if (task.Length() > 0)
			json_response["body"]["data"] = Utils::base64_encode(task.Data(), task.Length());
		break;
	}
	}

	DWORD dwRes = SendReply(json_response);

	if (dwRes != ERROR_SUCCESS)
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
			{"is_success", true},
			{"gle", ERROR_SUCCESS},
			{"type", TaskType::GetInterceptedIrps},
		}
	} };

	//
	// Make sure no element are being added concurrently
	//
	std::unique_lock<std::mutex> mlock(m_Session.m_IrpMutex);
	size_t i = 0;

	j["body"]["intercepted_irps"]["irps"] = json::array();

	while(!m_Session.m_IrpQueue.empty() && i < CFB_FRONTEND_MAX_ENTRIES)
	{
		//
		// pop an IRP
		//
		Irp irp(m_Session.m_IrpQueue.front());
		m_Session.m_IrpQueue.pop();

		//
		// format a new JSON entry
		//
		j["body"]["intercepted_irps"]["irps"].push_back(irp.ToJson());

		i++;
	}

	mlock.unlock();

	j["body"]["intercepted_irps"]["nb_irps"] = i;


	//
	// Write the data back
	//

	return SendReply(j);
}



/*++

Routine Description:

	Sends the list of drivers

Arguments:

	None

Return Value:

	Returns a ERROR_SUCCESS, or the corresponding WinError otherwise.

--*/
DWORD FrontEndServer::SendDriverList()
{
	json j = {
		{"header", {
			{"is_success", true},
			{"gle", ERROR_SUCCESS},
			{"type", TaskType::EnumerateDrivers}
		}
	}};

	j["body"]["driver_list"]["drivers"] = json::array();
	
	size_t nb_drivers = 0;
	std::vector<std::wstring> roots = { L"\\Driver" , L"\\FileSystem" };
	for (auto root : roots)
	{
		for (auto driver : Utils::EnumerateObjectDirectory(root))
		{
			std::wstring driver_abspath = root + std::wstring(L"\\") + driver.first;
			std::string driver_name = Utils::WideStringToString(driver_abspath);
			j["body"]["driver_list"]["drivers"].push_back(driver_name);
			nb_drivers++;
		}
	}

	j["body"]["driver_list"]["nb_drivers"] = nb_drivers;

	return SendReply(j);
}



/*++

Routine Description:

	Sends a IOCTL to the specified device, providing all the arguments from the JSON requests

Arguments:

	Task - 

Return Value:

	Returns a ERROR_SUCCESS, or the corresponding WinError otherwise.

--*/
DWORD FrontEndServer::SendForgedIrp(json& json_request)
{
	auto decoded_body = Utils::base64_decode(json_request["body"]["param"]);
	decoded_body.push_back(0);
	auto json_body = json::parse(decoded_body)["replay_irp"];

	auto DeviceName = json_body["device_name"].get<std::string>();
	auto IoctlCode = json_body["ioctl_code"];
	auto InputBufferLength = json_body["input_buffer_length"];
	auto OutputBufferLength = json_body["output_buffer_length"].get<DWORD>();
	auto InputBuffer = Utils::base64_decode(json_body["input_buffer"]);


	std::vector<byte> OutputBuffer;
	OutputBuffer.resize(OutputBufferLength);

	DWORD dwRes = Utils::Io::DeviceIoControlWrapper(
		DeviceName.c_str(),
		IoctlCode,
		InputBuffer.data(),
		InputBufferLength,
		OutputBuffer.data(),
		&OutputBufferLength
	);


	json json_response;

	json_response["header"]["is_success"] = dwRes == ERROR_SUCCESS;
	json_response["header"]["gle"] = dwRes;
	json_response["header"]["type"] = TaskType::ReplayIrp;

	json_response["body"]["replay_irp"]["output_buffer"] = OutputBuffer;
	json_response["body"]["replay_irp"]["output_buffer_length"] = OutputBufferLength;

	return SendReply(json_response);
}


DWORD FrontEndServer::SendOsInfo()
{
	//
	// OS version
	//
	OSVERSIONINFOW VersionInformation = { 0 };
	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	RtlGetVersion(&VersionInformation);

	DWORD dwVersionMajor, dwVersionMinor;
	dwVersionMajor = VersionInformation.dwMajorVersion;
	dwVersionMinor = VersionInformation.dwMinorVersion;

	std::wstring VersionBuild;
	Utils::Registry::ReadWString(
		HKEY_LOCAL_MACHINE,
		std::wstring(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
		std::wstring(L"CurrentBuildNumber"),
		VersionBuild
	);


	//
	// CPU info
	//
	SYSTEM_INFO si = { 0 };
	::GetNativeSystemInfo(&si);


	//
	// Process info
	//
	DWORD dwUserNameLength = MAX_USERNAME_SIZE;
	UCHAR lpBufferUserName[MAX_USERNAME_SIZE + 1] = { 0 };
	if (!::GetUserNameA((LPSTR)lpBufferUserName, &dwUserNameLength))
	{
		dwUserNameLength = sizeof("<unknown>");
		::RtlCopyMemory(lpBufferUserName, "<unknown>", dwUserNameLength);
	}

	std::string UserName(reinterpret_cast<char const*>(lpBufferUserName), dwUserNameLength-1);

	std::wstring IntegrityLevelName;
	if (Utils::Process::GetIntegrityLevel(IntegrityLevelName) != ERROR_SUCCESS)
		IntegrityLevelName = L"<unknown>";

	DWORD dwProcessId = ::GetCurrentProcessId();

	//
	// build and send response
	//
	json json_response;

	json_response["header"]["is_success"] = true;
	json_response["header"]["gle"] = ERROR_SUCCESS;
	json_response["header"]["type"] = TaskType::GetOsInfo;

	json_response["body"]["os_info"]["version_major"] = dwVersionMajor;
	json_response["body"]["os_info"]["version_minor"] = dwVersionMinor;
	json_response["body"]["os_info"]["version_build"] = Utils::WideStringToString(VersionBuild);

	json_response["body"]["os_info"]["cpu_num"] = si.dwNumberOfProcessors; 
	json_response["body"]["os_info"]["cpu_arch"] = si.wProcessorArchitecture;
	
	json_response["body"]["os_info"]["version"] = CFB_VERSION;
	json_response["body"]["os_info"]["username"] = UserName;
	json_response["body"]["os_info"]["pid"] = dwProcessId;
	json_response["body"]["os_info"]["integrity"] = Utils::WideStringToString(IntegrityLevelName);

	return SendReply(json_response);
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

