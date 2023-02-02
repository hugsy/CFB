#pragma once

// clang-format off
#include "Common.hpp"
#include "Broker.hpp"
#include "Error.hpp"
#include "ManagerBase.hpp"
#include "Messages.hpp"

#include <wil/resource.h>

#include <nlohmann/json.hpp>
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

        ~TcpClient();

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

        std::string
        Name();

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

        std::string
        Name();


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

    usize m_RequestNumber;
};

} // namespace CFB::Broker
