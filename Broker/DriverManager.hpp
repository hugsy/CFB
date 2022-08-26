#pragma once

// clang-format off
#include "Common.hpp"
#include "Broker.hpp"
#include "Error.hpp"
#include "ManagerBase.hpp"
#include "Messages.hpp"

#include <wil/resource.h>

#include "json.hpp"
using json = nlohmann::json;
// clang-format on

namespace CFB::Broker
{

class DriverManager : public ManagerBase
{
public:
    class TcpClient
    {
    public:
        TcpClient() : m_Id {0}, m_Socket {0}, m_IpAddress {}, m_Port {0}, m_ThreadId {0}, m_hThread {nullptr}
        {
        }

        /*
        TcpClient(const usize id, const SOCKET socket, std::string const& host_ip, const u16 host_port) :
            m_Id {id},
            m_Socket {socket},
            m_IpAddress {host_ip},
            m_Port {host_port},
            m_ThreadId {0},
            m_hThread {nullptr}
        {
        }

        ///
        /// @brief Move-Construct a new TcpClient object
        ///
        /// @param src
        ///
        TcpClient(TcpClient&& other) :
            m_Id {other.m_Id},
            m_Socket {other.m_Socket},
            m_Port {other.m_Port},
            m_ThreadId {other.m_ThreadId}
        {
            m_IpAddress = std::move(other.m_IpAddress);
            m_hThread   = std::move(other.m_hThread);
        }
        */


        ~TcpClient();
        /*
                TcpClient&
                operator=(TcpClient&& other)
                {
                    m_Id        = other.m_Id;
                    m_Socket    = other.m_Socket;
                    m_IpAddress = other.m_IpAddress;
                    m_Port      = other.m_Port;
                    m_ThreadId  = other.m_ThreadId;
                    m_IpAddress = std::move(other.m_IpAddress);
                    m_hThread   = std::move(other.m_hThread);
                    return *this;
                }
        */

        ///
        /// @brief Synchronous send
        ///
        /// @return Result<u32>
        ///
        Result<u32>
        SendSynchronous(json const&);

        ///
        /// @brief
        ///
        /// @return Result<std::vector<u8>>
        ///
        Result<json>
        ReceiveSynchronous();

        usize m_Id      = 0;
        SOCKET m_Socket = 0;
        std::string m_IpAddress;
        u16 m_Port                   = 0;
        u32 m_ThreadId               = 0;
        wil::unique_handle m_hThread = nullptr;
    };


    class TcpListener
    {
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
        Result<std::shared_ptr<TcpClient>>
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
        /// @brief
        ///
        /// @return Result<u32>
        ///
        Result<u32>
        RunForever();

    private:
        SOCKET m_ServerSocket;

        std::vector<std::shared_ptr<TcpClient>> m_Clients;
    };


    ///
    /// @brief Construct a new Driver Manager object
    ///
    DriverManager();

    ///
    /// @brief Destroy the Driver Manager object
    ///
    ~DriverManager();

    ///
    /// @brief
    ///
    /// @return std::string const
    ///
    std::string const
    Name();

    ///
    /// @brief
    ///
    /// @return Result<bool>
    ///
    Result<bool>
    Setup();

    ///
    /// @brief
    ///
    void
    Run();

    ///
    /// @brief Execute command directly on the broker. This can be used to have the broker build commands directly
    /// to the driver.
    ///
    /// @return Result<json>
    ///
    Result<json>
    ExecuteCommand(json const& Request);

private:
    ///
    /// @brief Handle to the device
    ///
    wil::unique_handle m_hDevice;

    ///
    /// @brief For now only use TCP
    ///
    TcpListener m_Listener;

    ///
    /// @brief
    ///
    ///
    std::mutex m_ManagerLock;
};

} // namespace CFB::Broker
