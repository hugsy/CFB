#pragma once

#include "common.h"

#include <aclapi.h>
#include <psapi.h>
#include <iostream>
#include <locale>
#include <codecvt>

#include "task.h"

#define MAX_ACCEPTABLE_MESSAGE_SIZE 65534
#define MAX_MESSAGE_SIZE (MAX_ACCEPTABLE_MESSAGE_SIZE+2)


class Session;


enum class ServerState
{
	Disconnected = 0,
	Connecting = 1,
	ReadyToReadFromClient = 2,
	ReadyToReadFromServer = 3
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
	HANDLE GetHandle() { return m_hServer; }

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
	HANDLE GetHandle() { return (HANDLE)m_Socket; }

private:
	SOCKET m_Socket;
	WSADATA m_WsaData;
};



class FrontEndServer
{
public:
	FrontEndServer() noexcept(false);
	~FrontEndServer() noexcept(false);
	
	DWORD RunForever(_In_ Session& Sess);
	HANDLE TransportHandle();

private:
	PipeTransportManager m_Transport;
};


_Success_(return) BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter);


#include "taskmanager.h"
#include "CfbException.h"
#include "Session.h"

#include "json.hpp"
using json = nlohmann::json;