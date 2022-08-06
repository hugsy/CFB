#pragma once

// clang-format off
#include "Common.hpp"
#include "Broker.hpp"
#include "Error.hpp"
#include "ManagerBase.hpp"

#include <wil/resource.h>
// clang-format on

namespace CFB::Broker
{

class DriverManager : ManagerBase
{

    class Listener
    {
    public:
        virtual Result<bool>
        Listen() = 0;

        virtual Result<u32>
        SendSynchronous(const SOCKET ClientSocket, std::vector<u8> const&) = 0;

        virtual Result<std::vector<u8>>
        ReceiveSynchronous(const SOCKET ClientSocket) = 0;

        virtual Result<u32>
        RunForever() = 0;
    };

    class TcpListener : Listener
    {
        class TcpClient
        {
        public:
            ~TcpClient();

            const usize m_Id;
            const SOCKET m_Socket;
            const std::string m_IpAddress;
            const u16 m_Port;
            u32 m_ThreadId;
            HANDLE m_hThread;
        };

    public:
        ///
        /// @brief Construct a new Tcp Listener object
        ///
        TcpListener();

        ///
        /// @brief Destroy the Tcp Listener object
        ///
        ~TcpListener();

        ///
        /// @brief Create the listening socket
        ///
        /// @return Result<bool>
        ///
        Result<bool>
        Initialize();

        ///
        /// @brief
        ///
        /// @return Result<bool>
        ///
        Result<bool>
        Listen();

        ///
        /// @brief Accept a client socket
        ///
        /// @return SOCKET
        ///
        std::unique_ptr<TcpClient>
        Accept();

        ///
        /// @brief
        ///
        /// @return true
        /// @return false
        ///
        bool
        Reconnect();

        ///
        /// @brief
        ///
        /// @return true
        /// @return false
        ///
        Result<bool>
        Terminate();

        ///
        /// @brief Synchronous send
        ///
        /// @return Result<u32>
        ///
        Result<u32>
        SendSynchronous(const SOCKET ClientSocket, std::vector<u8> const&);

        ///
        /// @brief
        ///
        /// @return Result<std::vector<u8>>
        ///
        Result<std::vector<u8>>
        ReceiveSynchronous(const SOCKET ClientSocket);

        Result<u32>
        RunForever();

    private:
        SOCKET m_ServerSocket;

        std::vector<std::unique_ptr<TcpClient>> m_Clients;
    };

public:
    ///
    /// @brief Construct a new Driver Manager object
    ///
    DriverManager();

    ///
    /// @brief Destroy the Driver Manager object
    ///
    ~DriverManager();

    std::string const
    Name();

    ///
    /// @brief
    ///
    void
    Run();

    ///
    /// @brief Takes a request Task, and creates and send a valid DeviceIoControl() to the IrpDumper driver. The
    /// function also builds a response Task from the response of the DeviceIoControl().
    ///
    /// @return Result<u32>
    ///
    Result<u32>
    SendIoctl();

private:
    ///
    /// @brief Handle to the device
    ///
    wil::unique_handle m_hDevice;

    ///
    /// @brief For now only use TCP
    ///
    TcpListener m_Listener;
};

} // namespace CFB::Broker
