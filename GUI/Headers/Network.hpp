#pragma once
#define WIN32_LEAN_AND_MEAN

#pragma warning(push)
#pragma warning(disable : 4005) // Macro redefinitions
#include <winsock.h>
#pragma warning(pop)

#include "App.hpp"

namespace CFB::GUI::App
{


class Target
{
public:
    std::string Host {};
    u16 Port {};
    bool IsConnected {};

    bool
    Connect()
    {
        if ( IsConnected )
        {
            return true;
        }

        WSADATA WsaData {};
        if ( ::WSAStartup(MAKEWORD(2, 2), &WsaData) )
        {
            return false;
        }

        m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if ( m_Socket == INVALID_SOCKET )
        {
            ::WSACleanup();
            return false;
        }

        SOCKADDR_IN sa;
        sa.sin_addr.s_addr = ::inet_addr(Host.c_str());
        sa.sin_family      = AF_INET;
        sa.sin_port        = ::htons(Port);
        IsConnected        = ::connect(m_Socket, (PSOCKADDR)&sa, sizeof(SOCKADDR_IN)) != SOCKET_ERROR;

        return IsConnected;
    }

    bool
    Disconnect()
    {
        if ( !IsConnected )
        {
            return true;
        }

        IsConnected = (::closesocket(m_Socket) == 0) ? false : true;
        ::WSACleanup();
        return IsConnected;
    }

    template<typename T>
    bool
    Send(T const& Buffer)
    {
        if ( !IsConnected )
        {
            return false;
        }

        int iResult = send(m_Socket, (const char*)Buffer.data(), Buffer.size(), 0);
        if ( iResult == SOCKET_ERROR )
        {
            return false;
        }
        return true;
    }

    template<typename T>
    T
    Receive(usize BufferSize)
    {
        if ( !IsConnected )
        {
            return {};
        }

        T Buffer;
        Buffer.resize(BufferSize);
        int iResult = recv(m_Socket, (char*)Buffer.data(), BufferSize, 0);
        if ( iResult == SOCKET_ERROR )
        {
            return {};
        }

        return Buffer;
    }

private:
    SOCKET m_Socket;
};


} // namespace CFB::GUI::App
