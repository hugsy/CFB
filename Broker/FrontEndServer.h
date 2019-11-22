#pragma once

#include "common.h"

#include "task.h"

#define MAX_ACCEPTABLE_MESSAGE_SIZE 65534
#define MAX_MESSAGE_SIZE (MAX_ACCEPTABLE_MESSAGE_SIZE+2)


class Session;


enum class ServerState
{
	Disconnected,
	Connecting,
	ReadyToReadFromClient,
	ReadyToReadFromServer
};


/*++

Base class for the transport abstraction layer: all transport medium must inherit this class.
Sub-classes *must* have an IFS compatible connection handle to support ReadFile() and WriteFile()

--*/
class ServerTransportManager
{
protected:
	ServerState m_dwServerState = ServerState::Disconnected;

public:
	virtual BOOL Initialize() = 0;
	virtual BOOL Terminate() = 0;
	virtual BOOL Connect() = 0;
	virtual BOOL Reconnect() = 0;
	virtual DWORD RunForever(_In_ Session& CurrentSession) = 0;
	virtual BOOL SendSynchronous(_In_ const std::vector<byte>& data) = 0;
	virtual std::vector<byte> ReceiveSynchronous() = 0;
};


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


/*++

Communication via TCP socket.

--*/

#define TCP_LISTEN_PORT 1337
#define TCP_MAX_CONNECTIONS 10


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
	BOOL Accept(_Out_ SOCKET* NewClient);

	SOCKET m_Socket;
};



class FrontEndServer
{
public:
	FrontEndServer() noexcept(false);
	~FrontEndServer() noexcept(false);
	
	DWORD RunForever(_In_ Session& Sess) { return m_Transport.RunForever(Sess); }
	BOOL Send(_In_ const std::vector<byte>& data) { return m_Transport.SendSynchronous(data); }
	std::vector<byte> Receive() { return m_Transport.ReceiveSynchronous(); }

private:
	//PipeTransportManager m_Transport;
	TcpSocketTransportManager m_Transport;
};


_Success_(return) BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter);