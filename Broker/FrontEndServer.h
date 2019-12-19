#pragma once

#include "common.h"

#include "TcpSocketTransportManager.h"
#include "PipeTransportManager.h"

class Session;
class Task;

#include <optional>
#include <string>

//
// Max number of JSON entries that can be served at a time
//
#define CFB_FRONTEND_MAX_ENTRIES 32


class FrontEndServer
{
public:
	FrontEndServer(_In_ Session& Session) noexcept(false);
	~FrontEndServer() noexcept(false);
	
	DWORD RunForever() { return m_Transport.RunForever(m_Session); }
	BOOL Send(_In_ const std::vector<byte>& data) { return m_Transport.SendSynchronous(data); }
	std::vector<byte> Receive() { return m_Transport.ReceiveSynchronous(); }
	std::vector<Task> ProcessNextRequest();
	BOOL ForwardReply();


private:
	//PipeTransportManager m_Transport;
	TcpSocketTransportManager m_Transport;
	Session& m_Session;
	std::vector<byte> m_ReceivedBytes;


	Task ProcessJsonTask(const std::string& json_request_as_string);
	std::optional<std::string> GetNextJsonStringMessage();


	//
	// Commands handled by the FrontEndServer, and not passed to the backend driver
	//
	DWORD SendInterceptedIrps();
	DWORD SendDriverList();
};


_Success_(return) BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter);