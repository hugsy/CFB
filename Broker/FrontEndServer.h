#pragma once

#include "common.h"
#include "task.h"

#include <aclapi.h>


enum ServerState : uint16_t
{
	Disconnected,
	Connecting,
	ReadyToReadFromClient,
	ReadyToReadFromServer
};


class ServerTransportManager
{
public:
	HANDLE m_hServer = INVALID_HANDLE_VALUE;
	ServerState m_dwServerState = ServerState::Disconnected;
};


class PipeTransportManager : public ServerTransportManager
{
public:
	OVERLAPPED m_oOverlap;
	bool m_fPendingIo = false;
};


class FrontEndServer
{

public:
	FrontEndServer() noexcept(false);
	~FrontEndServer() noexcept(false);
	
	PipeTransportManager m_Transport;

	BOOL ConnectPipe();
	BOOL DisconnectAndReconnectPipe();

private:
	BOOL CreatePipe();
	BOOL ClosePipe();
};


_Success_(return) BOOL StartFrontendManagerThread(_In_ LPVOID lpParameter);


#include "taskmanager.h"
#include "Session.h"
#include "CfbException.h"

#include "json.hpp"
using json = nlohmann::json;