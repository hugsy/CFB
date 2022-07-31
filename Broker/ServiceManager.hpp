#pragma once

#include <wil/resource.h>

#include <filesystem>
#include <mutex>
#include <optional>

#include "Common.hpp"
#include "ManagerBase.hpp"

namespace fs = std::filesystem;

namespace CFB::Broker
{

class Win32Service
{
    enum class ServiceState
    {
        Uninitialized,
        Initialized,
        Running,
        ShuttingDown,
        Shutdown
    };


public:
    ///
    /// @brief Construct a new Win32Service object
    ///
    Win32Service();

    ///
    /// @brief Destroy the Win32Service object
    ///
    ~Win32Service();

    ///
    /// @brief Run forever until told to stop
    ///
    void
    RunForever();

    ///
    /// @brief Stops the service, this method is invoked from the SCM
    ///
    /// @return true
    /// @return false
    ///
    bool
    Stop();

private:
    ///
    /// @brief
    ///
    /// @param lpServiceStatus
    /// @return true
    /// @return false
    ///
    bool
    ReportServiceStatus(LPSERVICE_STATUS lpServiceStatus);

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
    /// @brief Notification dispatcher
    ///
    /// @return true
    /// @return false
    ///
    bool Notify(ServiceState);

    ///
    /// @brief This mutex protects state changes
    ///
    std::mutex m_Mutex;

    ///
    /// @brief The manager current state
    ///
    ServiceState m_State;

    ///
    /// @brief Changed state notification event.
    ///
    HANDLE m_ServiceStateChangedEvent;

    ///
    /// @brief Handle to the service status
    ///
    SERVICE_STATUS_HANDLE m_StatusHandle;

    // SERVICE_STATUS m_ServiceStatus;

    usize m_StatusCheckPoint;
};

class ServiceManager : ManagerBase
{

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
    /// @return std::optional<Win32Service>&
    ///
    std::shared_ptr<Win32Service>
    BackgroundService();

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    InstallBackgroundService();

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    RunAsBackgroundService();

    void
    RunStandalone();

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
    /// @brief The fs::path of this binary
    ///
    fs::path m_BrokerPath;

    ///
    /// @brief The fs::path of the driver on disk
    ///
    fs::path m_DriverTempPath;

    ///
    /// @brief Unique pointer to the service control manager
    ///
    wil::unique_schandle m_hSCManager;

    ///
    /// @brief Unique pointer to the service manager
    ///
    wil::unique_schandle m_hService;

    ///
    /// @brief Background service (if set by globals)
    ///
    std::shared_ptr<Win32Service> m_BackgroundService;
};


} // namespace CFB::Broker
