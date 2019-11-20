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


class ServerTransportManager
{
public:
	HANDLE m_hServer = INVALID_HANDLE_VALUE;
	ServerState m_dwServerState = ServerState::Disconnected;

	virtual BOOL Initialize() { return true; }
	virtual BOOL Terminate() { return true; }

	virtual BOOL Connect() { return true; }
	virtual BOOL Reconnect() { return true; }

	virtual DWORD RunForever(_In_ Session& CurrentSession) 
	{
		dbg(L"not implemented");
		return 0;
	};
};


class PipeTransportManager : public ServerTransportManager
{
public:
	BOOL Initialize() { return CreatePipe(); }
	BOOL Terminate() { return ClosePipe(); }
	BOOL Connect() { return ConnectPipe(); }
	BOOL Reconnect() { return DisconnectAndReconnectPipe(); }
	DWORD RunForever(_In_ Session& CurrentSession);

private:
	OVERLAPPED m_oOverlap;
	bool m_fPendingIo = false;

	BOOL CreatePipe();
	BOOL ClosePipe();
	BOOL ConnectPipe();
	BOOL DisconnectAndReconnectPipe();
};


class FrontEndServer
{
public:
	FrontEndServer() noexcept(false);
	~FrontEndServer() noexcept(false);
	
	ServerTransportManager Transport();

private:
	ServerTransportManager m_Transport;
};


_Success_(return) BOOL FrontendThreadRoutine(_In_ LPVOID lpParameter);


#include "taskmanager.h"
#include "CfbException.h"
#include "Session.h"

#include "json.hpp"
using json = nlohmann::json;