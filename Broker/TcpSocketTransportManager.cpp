#include <WinSock2.h>
#include <ws2tcpip.h>

#include "TcpSocketTransportManager.h"
#include "Session.h"

#include "json.hpp"
using json = nlohmann::json;


TcpSocketTransportManager::TcpSocketTransportManager()
	: 
	m_ServerSocket(INVALID_SOCKET),
	m_ClientSocket(INVALID_SOCKET)
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

	m_ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (m_ServerSocket == INVALID_SOCKET)
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

	if (bind(m_ServerSocket, (PSOCKADDR)&sa, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		xlog(LOG_CRITICAL, L"Cannot bind socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		Terminate();
		return false;
	}

	if (listen(m_ServerSocket, SOMAXCONN_HINT(TCP_MAX_CONNECTIONS)))
	{
		xlog(LOG_CRITICAL, L"Cannot listen socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		Terminate();
		return false;
	}

	return true;
}


BOOL TcpSocketTransportManager::Terminate()
{
	return closesocket(m_ServerSocket) == 0;
}


BOOL TcpSocketTransportManager::Reconnect()
{
	return Terminate() && Initialize() && Connect();
}


/*++

Accept a client socket

--*/
SOCKET TcpSocketTransportManager::Accept()
{
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN SockInfoClient = { 0 };
	INT dwSockInfoClientSize = sizeof(SOCKADDR_IN);

	ClientSocket = ::WSAAccept(m_ServerSocket, (SOCKADDR*)&SockInfoClient, &dwSockInfoClientSize, NULL, 0); // todo: add a lpfnCondition
	if (ClientSocket == INVALID_SOCKET)
	{
		xlog(LOG_ERROR, L"Cannot accept from server socket (WSAGetLastError=0x%x)\n", ::WSAGetLastError());
		return INVALID_SOCKET;
	}

#ifdef _DEBUG
	WCHAR lpswIpClient[256] = { 0, };
	InetNtopW(AF_INET, &SockInfoClient.sin_addr.s_addr, lpswIpClient, _countof(lpswIpClient));
	dbg(L"New TCP client %s:%d\n", lpswIpClient, ntohs(SockInfoClient.sin_port));
#endif
	m_dwServerState = ServerState::Connecting;
	return ClientSocket;
}



/*++

Synchronous send for TCP streams

--*/
BOOL TcpSocketTransportManager::SendSynchronous(_In_ const std::vector<byte>& data)
{
	if (m_ClientSocket == INVALID_SOCKET)
		RAISE_GENERIC_EXCEPTION("not ready");
	/*
	if (data.size() >= MAX_ACCEPTABLE_MESSAGE_SIZE)
		return false;
	*/
	DWORD dwNbSentBytes = 0, dwFlags = 0;
	WSABUF DataBuf = { 0 };
	DataBuf.len = (DWORD)data.size();
	DataBuf.buf = (char*)data.data();

	if (::WSASend(m_ClientSocket, &DataBuf, 1, &dwNbSentBytes, dwFlags, NULL, NULL) == SOCKET_ERROR)
		RAISE_GENERIC_EXCEPTION("SendSynchronous");

	return true;
}


/*++

Synchronous receive for TCP streams

--*/
std::vector<byte> TcpSocketTransportManager::ReceiveSynchronous()
{
	if(m_ClientSocket == INVALID_SOCKET)
		RAISE_GENERIC_EXCEPTION("ReceiveSynchronous - not ready: ");

	auto buf = std::make_unique<byte[]>(MAX_MESSAGE_SIZE);
	RtlZeroMemory(buf.get(), MAX_MESSAGE_SIZE);
	DWORD dwNbRecvBytes = 0, dwFlags = 0;
	WSABUF DataBuf = { 0 };
	DataBuf.len = MAX_MESSAGE_SIZE;
	DataBuf.buf = (char*)buf.get();

	if (::WSARecv(m_ClientSocket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, NULL, NULL) == SOCKET_ERROR)
		RAISE_GENERIC_EXCEPTION("ReceiveSynchronous - recv: ");

	std::vector<byte> res;
	for (DWORD i = 0; i < dwNbRecvBytes; i++) res.push_back(buf[i]);
	return res;
}


/*++

This function handles the client (GUI) session 

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
	if (hEvent==WSA_INVALID_EVENT || ::WSAEventSelect(ClientSocket, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		DWORD gle = ::WSAGetLastError();
		xlog(LOG_ERROR, L"Cannot create event socket (WSAGetLastError=0x%x)\n", gle);
		return gle;
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

		if (dwIndexObject != 1)
		{
			if (dwIndexObject == 0)
			{
				dbg(L"received termination event\n");
				Sess.Stop();
			}
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
			fContinue = false;
			dwRetCode = ERROR_SUCCESS;
			continue;
		}

		try
		{
			//
			// if here, process the request
			//
			auto task = Sess.FrontEndServer.ProcessNextRequest();

			switch (task.Type())
			{
			case TaskType::GetInterceptedIrps:	continue;
			case TaskType::EnumerateDrivers:	continue;
			case TaskType::ReplayIrp:			continue;
			}

			//
			// and send the response
			//
			if (!Sess.FrontEndServer.ForwardReply())
				RAISE_GENERIC_EXCEPTION("ForwardReply");
		}
		catch (BaseException & e)
		{
			xlog(LOG_ERROR, L"exception %S\n", e.what());
			fContinue = false;
			dwRetCode = ERROR_INVALID_DATA;
			continue;
		}
	}

	::CloseHandle(hEvent);
	::closesocket(ClientSocket);
	
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
	if(::WSAEventSelect(m_ServerSocket, hEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		PrintErrorWithFunctionName(L"WSAEventSelect(TcpServer)");
		return ::WSAGetLastError();
	}
	handles.push_back(hEvent);


	while (CurrentSession.IsRunning())
	{
		SOCKET ClientSocket;
		HANDLE hClientThread;
		DWORD dwClientTid;

		DWORD dwIndexObject = ::WSAWaitForMultipleEvents((DWORD)handles.size(), handles.data(), false, INFINITE, false) - WSA_WAIT_EVENT_0;

		if (dwIndexObject < 0 || dwIndexObject >= handles.size())
		{
			PrintErrorWithFunctionName(L"WSAWaitForMultipleEvents(TcpServer)");
			CurrentSession.Stop();
			continue;
		}

		dbg(L"TcpServer: event triggered %d\n", dwIndexObject);

		switch (dwIndexObject)
		{
		case 0:
			dbg(L"received termination event\n");
			CurrentSession.Stop();
			break;

		case 1:
		{
			WSANETWORKEVENTS Events = { 0, };
			::WSAEnumNetworkEvents(m_ServerSocket, hEvent, &Events);

			if (Events.lNetworkEvents & FD_CLOSE)
			{
				// is a an TCP_CLOSE ?
				::CloseHandle(handles.at(dwIndexObject));
			}
			else
			{
				// it's a TCP_SYN, so accept the connection and spawn the thread handling the requests
				ClientSocket = Accept();
				if (ClientSocket == INVALID_SOCKET)
					continue;

				m_ClientSocket = ClientSocket;
				m_dwServerState = ServerState::ReadyToReadFromClient;

				PULONG_PTR args[2] = {
					(PULONG_PTR)&CurrentSession,
					(PULONG_PTR)m_ClientSocket
				};

				// start a thread to handle the requests
				hClientThread = ::CreateThread(nullptr, 0, HandleTcpRequestsRtn, args, 0, &dwClientTid);
				if (!hClientThread)
				{
					::closesocket(ClientSocket);
					m_ClientSocket = INVALID_SOCKET;
					PrintErrorWithFunctionName(L"CreateThread(TcpCli)");
					continue;
				}

				dbg(L"event on handle %d\n", dwIndexObject);
				handles.push_back(hClientThread);
			}
			break;
		}

		default:
			//
			// if here, we've received the event of EOL from the client thread, so we clean up and continue looping
			//
			dbg(L"default event_handle[%d], closing tcp client thread...\n", dwIndexObject);
			::CloseHandle(handles.at(dwIndexObject));
			handles.erase(handles.begin() + dwIndexObject);
			break;
		}
	}

	dbg(L"terminating thread TID=%d\n", ::GetThreadId(::GetCurrentThread()));
	return retcode;
}
