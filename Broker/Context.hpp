#pragma once

// clang-format off
#include <atomic>
#include <mutex>
#include <stop_token>
#include <thread>

#include "States.hpp"

#include "ServiceManager.hpp"
#include "IrpManager.hpp"
#include "ConnectorManager.hpp"
#include "DriverManager.hpp"
// clang-format on

namespace fs = std::filesystem;

class GlobalContext
{
public:
    ///
    /// @brief Construct a new Global Context object
    ///
    GlobalContext();

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    Stop();

    ///
    /// @brief Set new state
    ///
    /// @return true
    /// @return false
    ///
    bool SetState(CFB::Broker::State);

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    WaitForState(CFB::Broker::State WantedState);

    ///
    /// @brief
    ///
    /// @return CFB::Broker::State const
    ///
    CFB::Broker::State const
    State() const;

    ///
    /// @brief Read-only access to the current process ID
    ///
    /// @return u32 const
    ///
    u32 const
    Pid() const;

    ///
    /// @brief Read-only access to the broker path
    ///
    /// @return fs::path&
    ///
    fs::path const&
    Path() const;

    ///
    /// @brief Get a shared pointer to the service manager object
    ///
    /// @return std::shared_ptr<CFB::Broker::ServiceManager>
    ///
    std::shared_ptr<CFB::Broker::ServiceManager>
    ServiceManager() const;

    ///
    /// @brief
    ///
    /// @return const HANDLE
    ///
    const HANDLE
    TerminationEvent() const;

private:
    ///
    /// @brief This mutex protects state changes
    ///
    std::mutex m_StateMutex;

    ///
    /// @brief The manager current state
    ///
    CFB::Broker::State m_State;

    ///
    /// @brief The current process ID
    ///
    u32 m_Pid;

    ///
    /// @brief Broker path
    ///
    fs::path m_BrokerPath;

    ///
    /// @brief
    ///
    ///
    wil::unique_handle m_hTerminationEvent;

    //////////////////////////////////////////////////////////////////////////////
    ///
    /// Manager declaration below: each manager is its own jthread
    ///

    ///
    /// @brief A global thread interruption token: if set all the threads will attempt
    /// to exit cleanly
    ///
    std::stop_token m_InterruptToken;

    ///
    /// @brief The service manager self-extracts and creates the driver service
    ///
    std::jthread m_ServiceManagerThread;

    ///
    /// @brief
    ///
    std::jthread m_IrpManagerThread;

    ///
    /// @brief
    ///
    std::jthread m_ConnectorManagerThread;

    ///
    /// @brief
    ///
    std::jthread m_DriverManagerThread;

    ///
    /// @brief
    ///
    std::shared_ptr<CFB::Broker::ServiceManager> m_ServiceManager;

    ///
    /// @brief
    ///
    std::shared_ptr<CFB::Broker::IrpManager> m_IrpManager;

    ///
    /// @brief
    ///
    std::shared_ptr<CFB::Broker::ConnectorManager> m_ConnectorManager;

    ///
    /// @brief
    ///
    std::shared_ptr<CFB::Broker::DriverManager> m_DriverManager;
};


extern GlobalContext Globals;
