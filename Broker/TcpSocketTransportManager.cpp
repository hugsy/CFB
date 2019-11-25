#include <WinSock2.h>
#include <ws2tcpip.h>

#include "TcpSocketTransportManager.h"
#include "Session.h"

#include "json.hpp"
using json = nlohmann::json;


TcpSocketTransportManager::TcpSocketTransportManager()
	: m_Socket(INVALID_SOCKET)
{
	WSADATA WsaData = { 0, };

	if (WSAStartup(MAKEWORD(2, 2), &WsaData))
		RAISE_GENERIC_EXCEPTION("TcpSocketTransportManager::WSAStartup()");

	if (LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wVersion) != 2)
		RAISE_GENERIC_EXCEPTION("TcpSocketTransportManager - version check");
}


TcpSocketTransportManager::~TcpSocketTransportManager()
{
	WSACleanup();
}


/*++

Prepare the socket: init + bind + listen

--*/
BOOL TcpSocketTransportManager::Initialize()
{
	WSAPROTOCOL_INFO info = { 0, };
	info.dwServiceFlags1 |= XP1_IFS_HANDLES;

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (m_Socket == INVALID_SOCKET)
	{
		xlog(LOG_CRITICAL, L"Cannot create socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		return false;
	}

	return true;
}


/*++

Prepare the socket: bind + listen

--*/
BOOL TcpSocketTransportManager::Connect()
{
	SOCKADDR_IN sa = { 0, };
	InetPtonW(AF_INET, L"0.0.0.0", &sa.sin_addr.s_addr);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(TCP_LISTEN_PORT);

	if (bind(m_Socket, (PSOCKADDR)&sa, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		xlog(LOG_CRITICAL, L"Cannot bind socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		Terminate();
		return false;
	}

	if (listen(m_Socket, SOMAXCONN_HINT(TCP_MAX_CONNECTIONS)))
	{
		xlog(LOG_CRITICAL, L"Cannot listen socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		Terminate();
		return false;
	}

	return true;
}


BOOL TcpSocketTransportManager::Terminate()
{
	return closesocket(m_Socket) == 0;
}


BOOL TcpSocketTransportManager::Reconnect()
{
	return Terminate() && Initialize() && Connect();
}


/*++

Accept a client socket

--*/
BOOL TcpSocketTransportManager::Accept(_Out_ SOCKET NewClient)
{
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN SockInfoClient = { 0 };
	INT dwSockInfoClientSize = sizeof(SOCKADDR_IN);

	ClientSocket = WSAAccept(m_Socket, (SOCKADDR*)&SockInfoClient, &dwSockInfoClientSize, NULL, 0); // todo: add a lpfnCondition
	if (ClientSocket == INVALID_SOCKET)
	{
		xlog(LOG_ERROR, L"Cannot accept from server socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		return false;
	}

	dbg(L"New TCP client %s:%d\n", SockInfoClient.sin_addr.s_addr, ntohs(SockInfoClient.sin_port));
	NewClient = ClientSocket;
	m_dwServerState = ServerState::Connecting;
	return true;
}



/*++

Synchronous send for TCP streams

--*/
BOOL TcpSocketTransportManager::SendSynchronous(_In_ const std::vector<byte>& data)
{
	if (data.size() >= MAX_ACCEPTABLE_MESSAGE_SIZE)
		return false;

	DWORD dwNbSentBytes = 0, dwFlags = 0;
	WSABUF DataBuf = { 0 };
	DataBuf.len = (DWORD)data.size();
	DataBuf.buf = (char*)data.data();

	if (::WSASend(m_Socket, &DataBuf, 1, &dwNbSentBytes, dwFlags, nullptr, nullptr) == SOCKET_ERROR)
		RAISE_GENERIC_EXCEPTION("SendSynchronous");

	return true;
}


/*++

Synchronous receive for TCP streams

--*/
std::vector<byte> TcpSocketTransportManager::ReceiveSynchronous()
{
	auto buf = std::make_unique<byte[]>(MAX_MESSAGE_SIZE);
	RtlZeroMemory(buf.get(), MAX_MESSAGE_SIZE);
	DWORD dwNbRecvBytes = 0, dwFlags = 0;
	WSABUF DataBuf = { 0 };
	DataBuf.len = MAX_MESSAGE_SIZE;
	DataBuf.buf = (char*)buf.get();

	if (::WSARecv(m_Socket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, nullptr, nullptr) == SOCKET_ERROR)
		RAISE_GENERIC_EXCEPTION("ReceiveSynchronous");

	std::vector<byte> res;
	for (DWORD i = 0; i < dwNbRecvBytes; i++) res.push_back(buf[i]);
	return res;
}


/*++

This function handles one client only (i.e. one TCP connection)

--*/
static DWORD HandleTcpRequestsRtn(_In_ LPVOID lpParameter)
{
	PULONG_PTR lpParameters = (PULONG_PTR)lpParameter;
	Session& Sess = *(reinterpret_cast<Session*>(lpParameters[0]));
	SOCKET ClientSocket = (SOCKET)lpParameters[1];
	DWORD dwRetCode = ERROR_SUCCESS;

	dbg(L"in request handler\n");

	std::vector<HANDLE> handles;
	handles.push_back(Sess.m_hTerminationEvent);

	HANDLE hEvent = ::WSACreateEvent();
	if (::WSAEventSelect(ClientSocket, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		PrintErrorWithFunctionName(L"WSAEventSelect(TcpClient)");
		return ::WSAGetLastError();
	}

	handles.push_back(hEvent);
	bool fContinue = true;

	while (Sess.IsRunning() && fContinue)
	{
		DWORD dwIndexObject = ::WSAWaitForMultipleEvents((DWORD)handles.size(), handles.data(), false, INFINITE, false) - WSA_WAIT_EVENT_0;

		if (dwIndexObject < 0 || dwIndexObject >= handles.size())
		{
			PrintErrorWithFunctionName(L"WSAWaitForMultipleEvents(TcpClient)");
			fContinue = false;
			dwRetCode = ::WSAGetLastError();
			continue;
		}

		switch (dwIndexObject)
		{
		case 0:
			dbg(L"received termination event\n");
			Sess.Stop();
			fContinue = false;
			continue;

		case 1:
			break;

 		default:
			xlog(LOG_ERROR, L"fail\n");
			fContinue = false;
			continue;
		}

		//
		// handle the data sent / recv
		//

		WSANETWORKEVENTS Events = { 0, };
		::WSAEnumNetworkEvents(ClientSocket, hEvent, &Events);

		if (Events.lNetworkEvents & FD_CLOSE)
		{
			dbg(L"gracefully disconnecting %x\n", ClientSocket);
			::CloseHandle(hEvent);
			::closesocket(ClientSocket);
			fContinue = false;
			continue;
		}

		//
		// if here, it's a read/write
		// 
	}

	dbg(L"terminating thread TID=%d\n", ::GetThreadId(::GetCurrentThread()));
	return dwRetCode;
}


DWORD TcpSocketTransportManager::RunForever(_In_ Session& CurrentSession)
{
	if (!Initialize())
		return ERROR_INVALID_PARAMETER;

	if (!Connect())
		return ERROR_INVALID_PARAMETER;

	dbg(L"TCP socket ready\n");

	DWORD retcode = ERROR_SUCCESS;
	HANDLE hEvent = INVALID_HANDLE_VALUE;

	std::vector<HANDLE> handles;
	handles.push_back(CurrentSession.m_hTerminationEvent);

	hEvent = ::WSACreateEvent();
	if(::WSAEventSelect(m_Socket, hEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		PrintErrorWithFunctionName(L"WSAEventSelect(TcpServer)");
		return ::WSAGetLastError();
	}
	handles.push_back(hEvent);


	while (CurrentSession.IsRunning())
	{
		DWORD dwIndexObject = ::WSAWaitForMultipleEvents((DWORD)handles.size(), handles.data(), false, INFINITE, false) - WSA_WAIT_EVENT_0;

		if (dwIndexObject < 0 || dwIndexObject >= handles.size())
		{
			PrintErrorWithFunctionName(L"WSAWaitForMultipleEvents(TcpServer)");
			CurrentSession.Stop();
			continue;
		}

		SOCKET ClientSocket;
		HANDLE hClientThread;
		DWORD dwClientTid;

		dbg(L"TcpServer: event %d\n", dwIndexObject);

		switch (dwIndexObject)
		{
		case 0:
			dbg(L"received termination event\n");
			CurrentSession.Stop();
			break;

		case 1:
		{
			WSANETWORKEVENTS Events = { 0, };
			::WSAEnumNetworkEvents(m_Socket, hEvent, &Events);

			if (Events.lNetworkEvents & FD_CLOSE)
				// is a an TCP_CLOSE ?
				::CloseHandle(handles.at(dwIndexObject));
			
			else
			{
				// it's a TCP_SYN, so accept the connection and spawn the thread handling the requests
				if (!Accept(ClientSocket))
					continue;

				std::vector<PVOID> args(2);
				args.push_back((PVOID)&CurrentSession);
				args.push_back((PVOID)ClientSocket);

				// start a thread to handle the requests
				hClientThread = ::CreateThread(nullptr, 0, HandleTcpRequestsRtn, args.data(), 0, &dwClientTid);
				if (!hClientThread)
				{
					PrintErrorWithFunctionName(L"CreateThread");
				}
				else
				{
					dbg(L"event on handle %s\n", dwIndexObject);
					handles.push_back(hClientThread);
				}
			}
			break;
		}

		default:
			dbg(L"event on handle %s\n", dwIndexObject);
			::CloseHandle(handles.at(dwIndexObject));
			break;
		}
	}

	dbg(L"terminating thread TID=%d\n", ::GetThreadId(::GetCurrentThread()));
	return retcode;
}
