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

        const usize m_Id;
        const SOCKET m_Socket;
        const std::string m_IpAddress;
        const u16 m_Port;
        u32 m_ThreadId;
        HANDLE m_hThread;
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
        std::shared_ptr<TcpClient>
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
    /// @brief Takes a request Task, and creates and send a valid DeviceIoControl() to the IrpDumper driver. The
    /// function also builds a response Task from the response of the DeviceIoControl().
    ///
    /// @return Result<json>
    ///
    Result<json>
    ExecuteCommand(json const& Request);

    Result<CFB::Comms::DriverResponse>
    SendIoctlRequest(CFB::Comms::DriverRequest const& msg);


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
