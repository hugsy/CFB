#pragma once

#include "common.h"

#include <vector>

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