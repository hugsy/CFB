// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>

#include "DriverManager.hpp"
#include "Context.hpp"
#include "Log.hpp"

#include "json.hpp"
using json = nlohmann::json;
// clang-format on

#define CFB_BROKER_TCP_LISTEN_HOST L"0.0.0.0"
#define CFB_BROKER_TCP_LISTEN_PORT 1337
#define CFB_BROKER_TCP_MAX_CONNECTIONS 16
#define CFB_BROKER_TCP_MAX_MESSAGE_SIZE 1024

static usize TotalClientCounter = 0;

namespace CFB::Broker
{

DriverManager::TcpListener::TcpListener() : m_ServerSocket(INVALID_SOCKET)
{
    WSADATA WsaData = {0};

    //
    // Start up the socket subsystem
    //
    {
        if ( ::WSAStartup(MAKEWORD(2, 2), &WsaData) )
        {
            CFB::Log::perror("DriverManager::TcpListener::WSAStartup()");
            throw std::runtime_error("TcpListener()");
        }

        if ( LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wVersion) != 2 )
        {
            err("DriverManager::TcpListener - version check");
            throw std::runtime_error("TcpListener()");
        }
    }
}


DriverManager::TcpListener::~TcpListener()
{
    Terminate();
}


Result<bool>
DriverManager::TcpListener::Initialize()
{
    WSAPROTOCOL_INFO info = {0};
    info.dwServiceFlags1 |= XP1_IFS_HANDLES;

    m_ServerSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_NO_HANDLE_INHERIT);
    if ( m_ServerSocket == INVALID_SOCKET )
    {
        CFB::Log::perror("WSASocket()");
        return Err(ErrorCode::SocketInitializationFailed);
    }

    return Ok(true);
}


Result<bool>
DriverManager::TcpListener::Listen()
{
    SOCKADDR_IN sa = {0};
    ::InetPtonW(AF_INET, CFB_BROKER_TCP_LISTEN_HOST, &sa.sin_addr.s_addr);

    sa.sin_family = AF_INET;
    sa.sin_port   = ::htons(CFB_BROKER_TCP_LISTEN_PORT);

    if ( ::bind(m_ServerSocket, (PSOCKADDR)&sa, sizeof(SOCKADDR_IN)) == SOCKET_ERROR )
    {
        CFB::Log::perror("TcpListener::Listen::bind()");
        Terminate();
        return Err(ErrorCode::SocketInitializationFailed);
    }

    if ( ::listen(m_ServerSocket, SOMAXCONN_HINT(CFB_BROKER_TCP_MAX_CONNECTIONS)) )
    {
        CFB::Log::perror("TcpListener::Listen::listen()");
        Terminate();
        return Err(ErrorCode::SocketInitializationFailed);
    }

    return Ok(true);
}


Result<bool>
DriverManager::TcpListener::Terminate()
{
    bool res = (::closesocket(m_ServerSocket) == 0);
    ::WSACleanup();
    return Ok(res);
}


bool
DriverManager::TcpListener::Reconnect()
{
    return Success(Terminate()) && Success(Initialize()) && Success(Listen());
}


int CALLBACK
ConditionAcceptFunc(
    LPWSABUF lpCallerId,
    LPWSABUF lpCallerData,
    LPQOS pQos,
    LPQOS lpGQOS,
    LPWSABUF lpCalleeId,
    LPWSABUF lpCalleeData,
    GROUP FAR* g,
    DWORD_PTR dwCallbackData)
{
    dbg("TODO");
    return CF_ACCEPT;
}


std::unique_ptr<DriverManager::TcpListener::TcpClient>
DriverManager::TcpListener::Accept()
{
    SOCKET ClientSocket        = INVALID_SOCKET;
    SOCKADDR_IN SockInfoClient = {0};
    INT dwSockInfoClientSize   = sizeof(SOCKADDR_IN);

    ClientSocket =
        ::WSAAccept(m_ServerSocket, (SOCKADDR*)&SockInfoClient, &dwSockInfoClientSize, ConditionAcceptFunc, 0);
    if ( ClientSocket == INVALID_SOCKET )
    {
        err("WSAAccept() failed with WSAGetLastError=%#x", ::WSAGetLastError());
        return nullptr;
    }

    char Ipv4AddressClient[16] = {
        0,
    };
    ::InetNtopA(AF_INET, &SockInfoClient.sin_addr.s_addr, Ipv4AddressClient, _countof(Ipv4AddressClient));

    auto Client = std::make_unique<TcpClient>(
        TotalClientCounter++,
        ClientSocket,
        Ipv4AddressClient,
        ::ntohs(SockInfoClient.sin_port),
        -1,
        INVALID_HANDLE_VALUE);

    ok("New TCP client %s:%d (handle=%d)", Client->m_IpAddress, Client->m_Port, Client->m_Socket);
    return Client;
}


Result<u32>
DriverManager::TcpListener::SendSynchronous(const SOCKET ClientSocket, std::vector<u8> const& data)
{
    if ( ClientSocket == INVALID_SOCKET )
    {
        return Err(ErrorCode::UnexpectedStateError);
    }

    DWORD dwNbSentBytes = 0, dwFlags = 0;
    WSABUF DataBuf = {0};
    DataBuf.len    = (DWORD)data.size();
    DataBuf.buf    = (char*)data.data();

    if ( ::WSASend(ClientSocket, &DataBuf, 1, &dwNbSentBytes, dwFlags, nullptr, nullptr) == SOCKET_ERROR )
    {
        CFB::Log::perror("WSASend");
        return Err(ErrorCode::NetworkError);
    }

    return Ok(dwNbSentBytes);
}


Result<std::vector<u8>>
DriverManager::TcpListener::ReceiveSynchronous(const SOCKET ClientSocket)
{
    if ( ClientSocket == INVALID_SOCKET )
    {
        return Err(ErrorCode::UnexpectedStateError);
    }

    auto ReceivedDataBuffer = std::make_unique<u8[]>(CFB_BROKER_TCP_MAX_MESSAGE_SIZE);
    RtlZeroMemory(ReceivedDataBuffer.get(), CFB_BROKER_TCP_MAX_MESSAGE_SIZE);
    DWORD dwNbRecvBytes = 0, dwFlags = 0;
    WSABUF DataBuf = {0};
    DataBuf.len    = CFB_BROKER_TCP_MAX_MESSAGE_SIZE;
    DataBuf.buf    = (char*)ReceivedDataBuffer.get();

    int recv_result = ::WSARecv(ClientSocket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, NULL, NULL);
    if ( recv_result == SOCKET_ERROR )
    {
        //
        // check for overlapped data
        //
        if ( ::WSAGetLastError() != WSAEWOULDBLOCK )
        {
            CFB::Log::perror("WSARecv()");
            return Err(ErrorCode::NetworkError);
        }

        WSAOVERLAPPED Overlapped = {0};

        recv_result = ::WSARecv(ClientSocket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, &Overlapped, NULL);
        if ( ::WSAGetLastError() != WSA_IO_PENDING )
        {
            CFB::Log::perror("ReceiveSynchronous::WSARecv(Pending)");
            return Err(ErrorCode::NetworkError);
        }

        //
        // we are in Overlapped I/O but the thread really needs the data from the client to proceed,
        // so just wait
        //
        while ( true )
        {
            dwFlags       = 0;
            DWORD dwIndex = ::WSAWaitForMultipleEvents(1, &Overlapped.hEvent, true, WSA_INFINITE, false);
            if ( dwIndex == WSA_WAIT_FAILED || !Overlapped.hEvent )
            {
                break;
            }
            ::WSAResetEvent(Overlapped.hEvent);
            ::WSAGetOverlappedResult(ClientSocket, &Overlapped, &dwNbRecvBytes, false, &dwFlags);
            ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));
            break;
        }
    }

    std::vector<u8> res;
    res.resize(dwNbRecvBytes);
    ::RtlCopyMemory(res.data(), ReceivedDataBuffer.get(), dwNbRecvBytes);
    return res;
}


u32
TcpClientRoutine(LPVOID lpParameter)
{
    DWORD dwRetCode         = ERROR_SUCCESS;
    PULONG_PTR lpParameters = (PULONG_PTR)lpParameter;
    SOCKET ClientSocket     = (SOCKET)lpParameters[0];

    std::vector<HANDLE> handles;
    handles.push_back(Globals.TerminationEvent());

    //
    // Create the socket notification event for read, write and disconnection
    //
    wil::unique_handle hEvent(::WSACreateEvent());
    if ( !hEvent )
    {
        DWORD gle = ::WSAGetLastError();
        CFB::Log::perror("WSACreateEvent()");
        return SOCKET_ERROR;
    }

    if ( ::WSAEventSelect(ClientSocket, hEvent.get(), FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR )
    {
        CFB::Log::perror("WSAEventSelect()");
        return SOCKET_ERROR;
    }

    handles.push_back(hEvent.get());

    while ( true )
    {
        DWORD dwIndex = ::WSAWaitForMultipleEvents((DWORD)handles.size(), handles.data(), false, INFINITE, false) -
                        WSA_WAIT_EVENT_0;

        if ( dwIndex < 0 || dwIndex >= handles.size() )
        {
            CFB::Log::perror("WSAWaitForMultipleEvents(TcpClient)");
            dwRetCode = ::WSAGetLastError();
            break;
        }

        //
        // reset the event
        //
        ::WSAResetEvent(handles.at(dwIndex));

        if ( dwIndex != 1 )
        {
            if ( dwIndex == 0 )
            {
                dbg("received termination event");
                Globals.Stop();
            }
            continue;
        }

        //
        // handle the data sent / recv
        //
        WSANETWORKEVENTS Events = {
            0,
        };
        if ( ::WSAEnumNetworkEvents(ClientSocket, hEvent.get(), &Events) == SOCKET_ERROR )
        {
            err("WSAEnumNetworkEvents() failed with 0x%x", ::WSAGetLastError());
            continue;
        }

        if ( Events.lNetworkEvents & FD_CLOSE )
        {
            dbg("gracefully disconnecting handle=0x%x", ClientSocket);
            dwRetCode = ERROR_SUCCESS;
            ok("TCP client handle=%d disconnected", ClientSocket);
            break;
        }

        try
        {
            if ( (Events.lNetworkEvents & FD_READ) != FD_READ )
            {
                continue;
            }

            //
            // if here, process the requests (there may be multiple task per read)
            //

            //
            // TODO:
            // 1. read the request from socket and handle it
            // 2. push it to the driver manager
            // 3. wait for response
            // 4. send back
            //

            // ReceiveSynchronous(ClientSocket)
        }
        catch ( ... )
        {
            err("exception caught");
            dwRetCode = ERROR_INVALID_DATA;
            break;
        }
    }

    return dwRetCode;
}


Result<u32>
DriverManager::TcpListener::RunForever()
{
    if ( Failed(Initialize()) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( Failed(Listen()) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    dbg("TCP socket ready");

    DWORD retcode        = ERROR_SUCCESS;
    HANDLE hNetworkEvent = ::WSACreateEvent();
    if ( ::WSAEventSelect(m_ServerSocket, hNetworkEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR )
    {
        CFB::Log::perror("DriverManager::TcpListener::RunForever::WSAEventSelect()");
        return ::WSAGetLastError();
    }

    while ( true )
    {
        //
        // Wait for an event either of termination, from the network or any client thread
        //
        std::vector<HANDLE> Handles {Globals.TerminationEvent(), hNetworkEvent};
        for ( auto const& Client : m_Clients )
        {
            Handles.push_back(Client->m_hThread);
        }

        DWORD dwIndex = ::WSAWaitForMultipleEvents((DWORD)Handles.size(), Handles.data(), false, INFINITE, false) -
                        WSA_WAIT_EVENT_0;

        if ( dwIndex < 0 || dwIndex >= Handles.size() )
        {
            CFB::Log::perror("DriverManager::TcpListener::RunForever::WSAWaitForMultipleEvents()");
            Globals.Stop();
            break;
        }

        ::WSAResetEvent(Handles.at(dwIndex));

        switch ( dwIndex )
        {
        case 0:
            dbg("[TcpListener] received termination event");
            break;

        case 1:
        {
            WSANETWORKEVENTS Events = {0};
            ::WSAEnumNetworkEvents(m_ServerSocket, hNetworkEvent, &Events);

            if ( Events.lNetworkEvents & FD_CLOSE )
            {
                //
                // if it's a TCP_CLOSE, find the client from the handle, and terminate it
                //
                HANDLE hTarget          = Handles.at(dwIndex);
                auto FindClientByHandle = [&hTarget](auto const& e)
                {
                    return e->m_hThread == hTarget;
                };

                auto erased = std::erase_if(m_Clients, FindClientByHandle);
                if ( erased != 1 )
                {
                    throw std::runtime_error("Unexpected size for found client");
                }
            }
            else
            {
                //
                // it's a TCP_SYN, so accept the connection and spawn the thread handling the requests
                //
                std::unique_ptr<TcpClient> Client = Accept();
                if ( Client == nullptr )
                {
                    continue;
                }

                PULONG_PTR args[1] = {(PULONG_PTR)Client->m_Socket};

                //
                // start a thread to handle the new connection
                //
                Client->m_hThread = ::CreateThread(
                    nullptr,
                    0,
                    (LPTHREAD_START_ROUTINE)TcpClientRoutine,
                    args,
                    0,
                    reinterpret_cast<LPDWORD>(&Client->m_ThreadId));
                if ( !Client->m_hThread )
                {
                    CFB::Log::perror("CreateThread(TcpClientRoutine)");
                    continue;
                }

                m_Clients.push_back(std::move(Client));
            }
            break;
        }

        default:
            //
            // if here, we've received the event of EOL from the client thread, so we clean up and continue looping
            //
            dbg("[TcpListener] default event_handle[%d], closing tcp client thread...", dwIndex);
            const HANDLE hTarget    = Handles.at(dwIndex);
            auto FindClientByHandle = [&hTarget](auto const& e)
            {
                return e->m_hThread == hTarget;
            };

            auto erased = std::erase_if(m_Clients, FindClientByHandle);
            break;
        }
    }

    return retcode;
}


DriverManager::TcpListener::TcpClient::~TcpClient()
{
    dbg("Terminating TcpClient(%d, TID=%d)", m_Id, m_ThreadId);
    ::closesocket(m_Socket);
    ::CloseHandle(m_hThread);
}


} // namespace CFB::Broker
