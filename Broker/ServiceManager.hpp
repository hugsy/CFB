#pragma once

#include <wil/resource.h>

#include <filesystem>
#include <mutex>

#include "Common.hpp"

namespace fs = std::filesystem;

namespace CFB::Broker
{

class ServiceManager
{
    enum class State
    {
        Uninitialized,
        Initialized,
        Running,
        ShuttingDown,
        Shutdown
    };

public:
    ///
    /// @brief Construct a new Service Manager:: Service Manager object
    ///
    ///
    ServiceManager();

    ///
    /// @brief Destroy the Service Manager:: Service Manager object
    ///
    ///
    ~ServiceManager();

    ///
    /// @brief
    ///
    /// @param lpServiceStatus
    /// @return true
    /// @return false
    ///
    bool
    UpdateStatus(LPSERVICE_STATUS lpServiceStatus);

    ///
    /// @brief Set the Status Handle object
    ///
    /// @param hServiceStatus
    /// @return true
    /// @return false
    ///
    bool
    InitializeRoutine();

    ///
    /// @brief Run forever until told to stop
    ///
    void
    RunForever();

private:
    ///
    /// @brief Extracts the IrpMonitor driver embedded in the PE resource section.
    ///
    /// @return `true` upon successful extraction of the driver from the resource of the driver
    /// @return `false` f any error occured.
    ///
    bool
    ExtractDriverFromResource();

    ///
    /// @brief Delete the driver extracted from the PE resources from the disk.
    ///
    /// @return `true` upon successful deletion of the driver from the disk.
    /// @return `false`
    ///
    bool
    DeleteDriverFromDisk();

    ///
    /// @brief Connects to the Windows Service Manager to create and start a service for the IrpMonitor driver.
    ///
    /// @return true if the service was successfully created, and the driver loaded
    /// @return false in any other case
    ///
    bool
    LoadDriver();

    ///
    /// @brief Unloads the driver and deletes the service from the Windows Service Manager.
    ///
    /// @return `true `if the driver was successfully unloaded
    /// @return `false` in any other case
    ///
    bool
    UnloadDriver();

    ///
    /// @brief Notification dispatcher
    ///
    /// @return true
    /// @return false
    ///
    bool Notify(State);

    ///
    /// @brief Start the broker background service thread
    ///
    /// @return true
    /// @return false
    ///
    bool
    StartBackgroundService();

    ///
    /// @brief This mutex protects state changes
    ///
    std::mutex m_Mutex;

    ///
    /// @brief The manager current state
    ///
    State m_State;

    ///
    /// @brief The fs::path of the driver on disk
    ///
    fs::path m_DriverTempPath;

    ///
    /// @brief Changed state notification event.
    ///
    HANDLE m_ServiceStateChangedEvent;

    ///
    /// @brief Handle to the service status
    ///
    SERVICE_STATUS_HANDLE m_StatusHandle;

    ///
    /// Unique pointer to the service control manager
    ///
    wil::unique_schandle m_hSCManager;

    ///
    /// Unique pointer to the service manager
    ///
    wil::unique_schandle m_hService;
};


} // namespace CFB::Broker
