// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>

#include "DriverManager.hpp"
#include "Context.hpp"
#include "Log.hpp"
#include "BrokerUtils.hpp"
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

    return CF_ACCEPT;
}


std::shared_ptr<DriverManager::TcpClient>
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

    char Ipv4AddressClient[16] = {0};
    ::InetNtopA(AF_INET, &SockInfoClient.sin_addr.s_addr, Ipv4AddressClient, _countof(Ipv4AddressClient));

    auto Client = std::make_shared<TcpClient>(
        TotalClientCounter++,
        ClientSocket,
        Ipv4AddressClient,
        ::ntohs(SockInfoClient.sin_port),
        -1,
        nullptr);

    ok("New TCP client %s:%d (handle=%#x)", Client->m_IpAddress.c_str(), Client->m_Port, Client->m_Socket);
    return Client;
}


Result<u32>
DriverManager::TcpClient::SendSynchronous(json const& js)
{
    if ( m_Socket == INVALID_SOCKET )
    {
        return Err(ErrorCode::UnexpectedStateError);
    }

    std::string data    = js.dump();
    DWORD dwNbSentBytes = 0, dwFlags = 0;
    WSABUF DataBuf = {0};
    DataBuf.len    = (DWORD)data.size();
    DataBuf.buf    = (char*)data.data();

    if ( ::WSASend(m_Socket, &DataBuf, 1, &dwNbSentBytes, dwFlags, nullptr, nullptr) == SOCKET_ERROR )
    {
        CFB::Log::perror("WSASend");
        return Err(ErrorCode::NetworkError);
    }

    return Ok(dwNbSentBytes);
}


Result<json>
DriverManager::TcpClient::ReceiveSynchronous()
{
    if ( m_Socket == INVALID_SOCKET )
    {
        return Err(ErrorCode::UnexpectedStateError);
    }

    auto ReceivedDataBuffer = std::make_unique<u8[]>(CFB_BROKER_TCP_MAX_MESSAGE_SIZE);
    RtlZeroMemory(ReceivedDataBuffer.get(), CFB_BROKER_TCP_MAX_MESSAGE_SIZE);
    DWORD dwNbRecvBytes = 0, dwFlags = 0;
    WSABUF DataBuf = {0};
    DataBuf.len    = CFB_BROKER_TCP_MAX_MESSAGE_SIZE;
    DataBuf.buf    = (char*)ReceivedDataBuffer.get();

    int recv_result = ::WSARecv(m_Socket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, NULL, NULL);
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

        recv_result = ::WSARecv(m_Socket, &DataBuf, 1, &dwNbRecvBytes, &dwFlags, &Overlapped, NULL);
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
            ::WSAGetOverlappedResult(m_Socket, &Overlapped, &dwNbRecvBytes, false, &dwFlags);
            ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));
            break;
        }
    }

    std::string s(ReceivedDataBuffer.get(), ReceivedDataBuffer.get() + dwNbRecvBytes);
    s.resize(dwNbRecvBytes);
    return json::parse(s);
}


u32
TcpClientRoutine(std::shared_ptr<DriverManager::TcpClient> pClient)
{
    DWORD dwRetCode = ERROR_SUCCESS;

    std::vector<HANDLE> handles;
    handles.push_back(Globals.TerminationEvent());

    std::shared_ptr<DriverManager::TcpClient> Client = pClient;

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

    if ( ::WSAEventSelect(Client->m_Socket, hEvent.get(), FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR )
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
        WSANETWORKEVENTS Events = {0};
        if ( ::WSAEnumNetworkEvents(Client->m_Socket, hEvent.get(), &Events) == SOCKET_ERROR )
        {
            err("WSAEnumNetworkEvents() failed with 0x%x", ::WSAGetLastError());
            continue;
        }

        if ( Events.lNetworkEvents & FD_CLOSE )
        {
            dbg("gracefully disconnecting handle=0x%x", Client->m_Socket);
            dwRetCode = ERROR_SUCCESS;
            ok("TCP client handle=%d disconnected", Client->m_Socket);
            break;
        }

        try
        {
            if ( (Events.lNetworkEvents & FD_READ) != FD_READ )
            {
                continue;
            }

            json Request, Response;
            //
            // 1. if here, process the requests
            //
            {
                auto res = Client->ReceiveSynchronous();
                if ( Failed(res) )
                {
                    warn("ReceiveSynchronous() failed with %x", Error(res).code);
                    continue;
                }

                Request = Value(res);
            }

            //
            // 2. let to the driver manager execute the command
            //
            {
                auto res = Globals.DriverManager()->ExecuteCommand(Request);
                if ( Failed(res) )
                {
                    Response["error_code"] = Error(res).code;
                }
                else
                {
                    Response["error_code"] = 0;
                    Response["body"]       = Value(res);
                }
            }

            //
            // 3. send the response back
            //
            Client->SendSynchronous(Response);
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

    DWORD retcode   = ERROR_SUCCESS;
    bool is_running = false;
    wil::unique_handle hNetworkEvent(::WSACreateEvent());
    if ( ::WSAEventSelect(m_ServerSocket, hNetworkEvent.get(), FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR )
    {
        CFB::Log::perror("DriverManager::TcpListener::RunForever::WSAEventSelect()");
        return ::WSAGetLastError();
    }

    is_running = true;

    while ( is_running )
    {
        //
        // Collect all events/handles to wait on
        //
        std::vector<HANDLE> Handles {Globals.TerminationEvent(), hNetworkEvent.get()};

        for ( auto const& cli : m_Clients )
        {
            Handles.push_back(cli->m_hThread.get());
        }

        //
        // Wait for an event either of termination, from the network or any client thread
        //
        DWORD ret = ::WSAWaitForMultipleEvents((DWORD)Handles.size(), Handles.data(), false, WSA_INFINITE, false);

        if ( ret == WSA_WAIT_FAILED || ret == WSA_WAIT_TIMEOUT )
        {
            CFB::Log::perror("DriverManager::TcpListener::RunForever::WSAWaitForMultipleEvents()");
            Globals.Stop();
            break;
        }

        DWORD dwIndex = ret - WSA_WAIT_EVENT_0;

        ::WSAResetEvent(Handles.at(dwIndex));

        switch ( dwIndex )
        {
        case 0:
            dbg("[TcpListener] Global stop requested...");
            is_running = false;
            break;

        case 1:
        {
            WSANETWORKEVENTS Events = {0};
            ::WSAEnumNetworkEvents(m_ServerSocket, hNetworkEvent.get(), &Events);

            if ( Events.lNetworkEvents & FD_CLOSE )
            {
                //
                // if it's a TCP_CLOSE, find the client from the handle, and terminate it
                //
                const HANDLE hTarget    = Handles.at(dwIndex);
                auto FindClientByHandle = [&hTarget](std::shared_ptr<TcpClient> const& cli) -> bool
                {
                    return cli->m_hThread.get() == hTarget;
                };

                auto erased = std::erase_if(m_Clients, FindClientByHandle);
                if ( erased != 1 )
                {
                    err("[TCP_CLOSE] Unexpected size for found client");
                }
                break;
            }

            if ( Events.lNetworkEvents & FD_ACCEPT )
            {
                //
                // it's a TCP_SYN, so accept the connection and spawn the thread handling the requests
                //
                std::shared_ptr<TcpClient> Client = Accept();
                if ( Client == nullptr )
                {
                    continue;
                }

                //
                // start a thread to handle the new connection
                //
                {
                    DWORD dwThreadId;
                    wil::unique_handle hThread(::CreateThread(
                        nullptr,
                        0,
                        (LPTHREAD_START_ROUTINE)TcpClientRoutine,
                        &Client,
                        THREAD_CREATE_FLAGS_CREATE_SUSPENDED,
                        reinterpret_cast<LPDWORD>(&dwThreadId)));
                    if ( !hThread )
                    {
                        CFB::Log::perror("CreateThread(TcpClientRoutine)");
                        continue;
                    }

                    Client->m_hThread  = std::move(hThread);
                    Client->m_ThreadId = dwThreadId;
                }

                ::ResumeThread(Client->m_hThread.get());
                m_Clients.push_back(Client);
                break;
            }

            err("DriverManager::TcpListener::RunForever::WSAEnumNetworkEvents(): unknown network event value %.08x",
                Events.lNetworkEvents);
            break;
        }

        default:
            //
            // if here, we've received the event of EOL from the client thread, so we clean up and continue looping
            //
            const HANDLE hTarget = Handles.at(dwIndex);
            dbg("[TcpListener] handling event_index=%d, thread_handle=%p", dwIndex, hTarget);

            //
            // Delete the entry in the client list
            //
            {
                auto FindClientByHandle = [&hTarget](std::shared_ptr<TcpClient> const& cli) -> bool
                {
                    return (cli->m_hThread.get() == hTarget);
                };

                auto const erased = std::erase_if(m_Clients, FindClientByHandle);
                if ( erased != 1 )
                {
                    warn("Unexpected size for found client, erased_nb = %d", erased);
                }
            }
        }
    }

    return retcode;
}


DriverManager::TcpClient::~TcpClient()
{
    dbg("Terminating TcpClient(Id=%d, TID=%d)", m_Id, m_ThreadId);
    ::closesocket(m_Socket);
}


} // namespace CFB::Broker
