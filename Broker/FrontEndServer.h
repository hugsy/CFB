#pragma once

#include "common.h"

#include "TcpSocketTransportManager.h"
#include "PipeTransportManager.h"

class Session;
class Task;


class FrontEndServer
{
public:
	FrontEndServer(_In_ Session& Session) noexcept(false);
	~FrontEndServer() noexcept(false);
	
	DWORD RunForever() { return m_Transport.RunForever(m_Session); }
	BOOL Send(_In_ const std::vector<byte>& data) { return m_Transport.SendSynchronous(data); }
	std::vector<byte> Receive() { return m_Transport.ReceiveSynchronous(); }
	Task ProcessNextRequest();
	BOOL ForwardReply();


	//
	// Commands handled by the FrontEndServer, and not passed to the backend driver
	//
	DWORD SendInterceptedIrps();
	DWORD SendDriverList();


private:
	//PipeTransportManager m_Transport;
	TcpSocketTransportManager m_Transport;
	Session& m_Session;
};


_Success_(return) BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter);