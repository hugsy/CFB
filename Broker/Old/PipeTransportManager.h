#pragma once


#include "common.h"
#include "ServerTransportManager.h"


class Session;


/*++

Communication via remote named pipe.

--*/
class PipeTransportManager : public ServerTransportManager
{
public:
	BOOL Initialize() { return CreatePipe(); }
	BOOL Terminate() { return ClosePipe(); }
	BOOL Connect() { return ConnectPipe(); }
	BOOL Reconnect() { return DisconnectAndReconnect(); }
	DWORD RunForever(_In_ Session& CurrentSession);
	BOOL SendSynchronous(_In_ const std::vector<byte>& data);
	std::vector<byte> ReceiveSynchronous();

private:
	HANDLE m_hServer;
	OVERLAPPED m_oOverlap;
	bool m_fPendingIo = false;

	BOOL CreatePipe();
	BOOL ClosePipe();
	BOOL ConnectPipe();
	BOOL DisconnectAndReconnect();
};
