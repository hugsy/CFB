#include "ServiceManager.hpp"

#include "Context.hpp"
#include "Log.hpp"


///
/// @brief
///
/// This class manages the IrpDumper driver stored in the PE resource section, and
/// defines all the functions to properly:
///
/// - extract the driver to disk
/// - create / delete the service
/// - load / unload the driver
/// - delete the PE file from disk
///
///
/// CFB can also be installed as a process service to facilitate automation, using sc.exe:
///     sc.exe create CFB_Broker binPath= "\path\to\Broker.exe --service" DisplayName= "Furious Beaver process service"
///
/// Then can be manipulated with the usual `sc start/stop`, and uninstalled with `sc delete`
///


namespace CFB::Broker
{
///
/// @brief Static handler that updates the service control manager's status information
///
/// @param dwCtrlCode
/// @return VOID
///
static VOID
ServiceCtrlHandler(const DWORD dwCtrlCode);

///
/// Static routine to initialize the own process service.
///
static VOID
ServiceMain(DWORD argc, LPWSTR* argv);


ServiceManager::ServiceManager() :
    m_DriverTempPath(fs::temp_directory_path() / CFB_DRIVER_BASENAME),
    m_StatusHandle(),
    m_State(State::Uninitialized)
{
    if ( ExtractDriverFromResource() == false )
    {
        throw std::runtime_error("ExtractDriverFromResource()");
    }

    if ( LoadDriver() == false )
    {
        throw std::runtime_error("LoadDriver()");
    }

    m_ServiceStateChangedEvent = ::CreateEventW(nullptr, true, false, nullptr);
    if ( !m_ServiceStateChangedEvent )
    {
        throw std::runtime_error("CreateEvent()");
    }
}


ServiceManager::~ServiceManager()
{
    if ( UnloadDriver() == false )
    {
        err("UnloadDriver() failed");
    }

    if ( DeleteDriverFromDisk() == false )
    {
        err("DeleteDriverFromDisk() failed");
    }
}


bool
ServiceManager::ExtractDriverFromResource()
{
    dbg("Extracting driver from resources...");

    HRSRC DriverRsc = ::FindResourceW(nullptr, MAKEINTRESOURCEW(CFB_BROKER_RC_DRIVER_NAME), CFB_BROKER_RC_DRIVER_ID);
    if ( !DriverRsc )
    {
        CFB::Log::perror("FindResource()");
        return false;
    }

    DWORD dwDriverSize = ::SizeofResource(nullptr, DriverRsc);
    if ( !dwDriverSize )
    {
        CFB::Log::perror("SizeofResource()");
        return false;
    }

    HGLOBAL hgDriverRsc = ::LoadResource(nullptr, DriverRsc);
    if ( !hgDriverRsc )
    {
        CFB::Log::perror("LoadResource()");
        return false;
    }

    dbg("Dumping driver to '%s'", m_DriverTempPath.c_str());

    wil::unique_handle hDriverFile(::CreateFileW(
        m_DriverTempPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr));
    if ( !hDriverFile )
    {
        CFB::Log::perror("CreateFile()");
        return false;
    }

    DWORD dwWritten;
    if ( !::WriteFile(hDriverFile.get(), hgDriverRsc, dwDriverSize, &dwWritten, nullptr) )
    {
        CFB::Log::perror("WriteFile()");
        return false;
    }

    if ( dwWritten != dwDriverSize )
    {
        err("Incomplete driver file dump");
        return false;
    }

    ok("Driver written in '%s'", m_DriverTempPath.c_str());
    return true;
}


bool
ServiceManager::DeleteDriverFromDisk()
{
    return m_DriverTempPath.remove_filename().has_filename() == false;
}


bool
ServiceManager::LoadDriver()
{
    dbg("Loading driver '%s'", m_DriverTempPath.c_str());

    //
    // Get a handle to the service control manager
    //
    {
        wil::unique_schandle hSCManager(::OpenSCManagerW(L"", SERVICES_ACTIVE_DATABASEW, SC_MANAGER_CREATE_SERVICE));
        if ( !hSCManager )
        {
            CFB::Log::perror("OpenSCManager()");
            return false;
        }

        m_hSCManager = std::move(hSCManager);
    }


    //
    // Get a handle to the service manager
    //
    {
        wil::unique_schandle hServiceCreate(::CreateServiceW(
            m_hSCManager.get(),
            CFB_BROKER_SERVICE_NAME,
            CFB_BROKER_SERVICE_DESCRIPTION,
            SERVICE_START | DELETE | SERVICE_STOP,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            m_DriverTempPath.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr));
        if ( !hServiceCreate )
        {
            if ( ::GetLastError() != ERROR_SERVICE_EXISTS )
            {
                //
                // Failure can mean the service already registered, if so just open it simply get a handle to it
                //
                CFB::Log::perror("CreateService()");
                return false;
            }
        }

        //
        // Try to open the service instead
        //
        wil::unique_schandle hServiceOpen(
            ::OpenServiceW(m_hSCManager.get(), CFB_BROKER_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP));
        {
            if ( !hServiceOpen )
                CFB::Log::perror("OpenService()");
            return false;
        }

        m_hService = std::move(hServiceOpen);
    }

    //
    // Start the service
    //
    dbg("Starting service '%S'", CFB_BROKER_SERVICE_NAME);

    if ( !::StartServiceW(m_hService.get(), 0, NULL) )
    {
        CFB::Log::perror("StartService()");
        return false;
    }

    ok("Service '%S' started successfully.");
    return true;
}


bool
ServiceManager::UnloadDriver()
{
    SERVICE_STATUS ServiceStatus = {0};

    dbg("Stopping service '%S'\n", CFB_BROKER_SERVICE_NAME);

    if ( !::ControlService(m_hService.get(), SERVICE_CONTROL_STOP, &ServiceStatus) )
    {
        CFB::Log::perror("ControlService()");
        return false;
    }

    dbg("Service '%S' stopped", CFB_BROKER_SERVICE_NAME);

    if ( !::DeleteService(m_hService.get()) )
    {
        CFB::Log::perror("DeleteService()");
        return false;
    }

    return true;
}


bool
ServiceManager::Notify(State NewState)
{
    {
        auto lock = std::scoped_lock(m_Mutex);
        m_State   = NewState;
    }

    if ( false == ::SetEvent(m_ServiceStateChangedEvent) )
    {
        CFB::Log::perror("SetEvent()");
        return false;
    }

    return true;
}


bool
ServiceManager::StartBackgroundService()
{
    auto lpswServiceName = (LPWSTR)CFB_BROKER_SERVICE_NAME;

    SERVICE_TABLE_ENTRYW ServiceTable[] = {
        {lpswServiceName, (LPSERVICE_MAIN_FUNCTIONW)ServiceMain},
        {nullptr, nullptr}};

    return ::StartServiceCtrlDispatcherW(ServiceTable);
}


bool
ServiceManager::UpdateStatus(LPSERVICE_STATUS lpServiceStatus)
{
    if ( !::SetServiceStatus(m_StatusHandle, lpServiceStatus) )
    {
        CFB::Log::perror("SetServiceStatus()");
        return false;
    }

    State NewState;

    switch ( lpServiceStatus->dwCurrentState )
    {
    case SERVICE_START_PENDING:
        NewState = State::Initialized;
        break;

    case SERVICE_RUNNING:
        NewState = State::Running;
        break;

    case SERVICE_STOP_PENDING:
        NewState = State::ShuttingDown;
        break;

    default:
        err("Unknown state");
        return false;
    }

    if ( !Notify(NewState) )
    {
        return false;
    }

    return true;
}


bool
ServiceManager::InitializeRoutine()
{
    //
    // Expect the state to be `Uninitialized`, otherwise just bail
    //
    if ( m_State != State::Uninitialized )
    {
        warn("Invalid state");
        return false;
    }

    //
    // Register the controller, and get the service status handle
    //
    {
        SERVICE_STATUS_HANDLE hServiceStatus =
            ::RegisterServiceCtrlHandlerW(CFB_BROKER_SERVICE_NAME, ServiceCtrlHandler);
        if ( !hServiceStatus )
        {
            CFB::Log::perror("RegisterServiceCtrlHandler()");
            return false;
        }

        m_StatusHandle = hServiceStatus;
    }

    //
    // Set the service in a start pending state
    //
    {
        SERVICE_STATUS ServiceStatus            = {0};
        ServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
        ServiceStatus.dwControlsAccepted        = 0;
        ServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
        ServiceStatus.dwWin32ExitCode           = 0;
        ServiceStatus.dwServiceSpecificExitCode = 0;
        ServiceStatus.dwCheckPoint              = 0;

        if ( UpdateStatus(&ServiceStatus) == false )
        {
            return false;
        }
    }

    //
    // Mark the service as running
    //
    {
        SERVICE_STATUS ServiceStatus     = {0};
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
        ServiceStatus.dwWin32ExitCode    = 0;
        ServiceStatus.dwCheckPoint       = 0;

        if ( UpdateStatus(&ServiceStatus) == false )
        {
            return false;
        }
    }

    return true;
}


void
ServiceManager::RunForever()
{
    //
    // Wait to be in a shutdown state
    //
    while ( true )
    {
        DWORD bRes = ::WaitForSingleObject(m_ServiceStateChangedEvent, INFINITE);
        if ( bRes == WAIT_OBJECT_0 )
        {
            auto lock = std::scoped_lock(m_Mutex);
            if ( m_State == State::ShuttingDown )
            {
                break;
            }
        }
    }

    //
    // Finish by notifying the manager
    //
    {
        SERVICE_STATUS ServiceStatus     = {0};
        ServiceStatus.dwControlsAccepted = 0;
        ServiceStatus.dwCurrentState     = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode    = 0;
        ServiceStatus.dwCheckPoint       = 3;

        if ( UpdateStatus(&ServiceStatus) == false )
        {
            CFB::Log::perror("SetServiceStatus() failed");
        }
    }
}


static VOID
ServiceCtrlHandler(const DWORD dwCtrlCode)
{
    if ( dwCtrlCode != SERVICE_CONTROL_STOP )
    {
        warn("Unhandled control code: 0x%x", dwCtrlCode);
        return;
    }

    SERVICE_STATUS ServiceStatus     = {0};
    ServiceStatus.dwControlsAccepted = 0;
    ServiceStatus.dwCurrentState     = SERVICE_STOP_PENDING;
    ServiceStatus.dwWin32ExitCode    = 0;
    ServiceStatus.dwCheckPoint       = 4;

    if ( Globals.ServiceManager.UpdateStatus(&ServiceStatus) == false )
    {
        CFB::Log::perror("SetServiceStatus()");
        return;
    }

    return;
}


static VOID
ServiceMain(DWORD argc, LPWSTR* argv)
{
    CFB::Broker::ServiceManager& ServiceManager = Globals.ServiceManager;

    //
    // This routine is called by the Windows Service Manager, finish the initialization of the object with the
    // information we got from it
    //
    if ( ServiceManager.InitializeRoutine() == false )
    {
        err("Failed to start the service");
        return;
    }

    dbg("%S background service ready, starting thread...", CFB_BROKER_SERVICE_NAME);

    //
    // All set up, now just run the service forever in background
    //
    ServiceManager.RunForever();

    return;
}


} // namespace CFB::Broker
