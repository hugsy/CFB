#pragma once

#include "common.h"
#include "ServerTransportManager.h"

class Session;


/*++

Communication via TCP socket.

--*/

#define TCP_LISTEN_PORT 1337
#define TCP_MAX_CONNECTIONS 1


class TcpSocketTransportManager : public ServerTransportManager
{
public:
	TcpSocketTransportManager();
	~TcpSocketTransportManager();

	BOOL Initialize();
	BOOL Terminate();
	BOOL Connect();
	BOOL Reconnect();
	DWORD RunForever(_In_ Session& CurrentSession);
	BOOL SendSynchronous(_In_ const std::vector<byte>& data);
	std::vector<byte> ReceiveSynchronous();


private:
	SOCKET Accept();

	SOCKET m_ServerSocket;
	SOCKET m_ClientSocket;
};
