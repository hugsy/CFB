#include "ServiceManager.hpp"

#include "Context.hpp"
#include "Log.hpp"
#include "Resource.h"


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
    m_BackgroundService(nullptr),
    m_DriverTempPath(fs::temp_directory_path() / CFB_DRIVER_BASENAME)
{
    if ( ExtractDriverFromResource() == false )
    {
        throw std::runtime_error("ExtractDriverFromResource()");
    }

    if ( LoadDriver() == false )
    {
        throw std::runtime_error("LoadDriver()");
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


std::shared_ptr<Win32Service>
ServiceManager::BackgroundService()
{
    return m_BackgroundService;
}


bool
ServiceManager::ExtractDriverFromResource()
{
    dbg("Extracting driver from resources...");

    HRSRC DriverRsc = ::FindResourceW(nullptr, MAKEINTRESOURCEW(IDR_CFB_DRIVER), MAKEINTRESOURCEW(CFB_DRIVER_DATAFILE));
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

    dbg("Dumping driver to '%S'", m_DriverTempPath.c_str());

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

    ok("Driver written in '%S'", m_DriverTempPath.c_str());
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
    dbg("Loading driver '%S'", m_DriverTempPath.c_str());

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
            CFB_BROKER_DRIVER_SERVICE_NAME,
            CFB_BROKER_DRIVER_SERVICE_DESCRIPTION,
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
            ::OpenServiceW(m_hSCManager.get(), CFB_BROKER_DRIVER_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP));
        {
            if ( !hServiceOpen )
            {
                CFB::Log::perror("OpenService()");
                return false;
            }
        }

        m_hService = std::move(hServiceOpen);
    }

    //
    // Start the service
    //
    dbg("Starting service '%S'", CFB_BROKER_DRIVER_SERVICE_NAME);

    if ( !::StartServiceW(m_hService.get(), 0, nullptr) )
    {
        CFB::Log::perror("StartService()");
        return false;
    }

    ok("Service '%S' started successfully.", CFB_BROKER_DRIVER_SERVICE_NAME);
    return true;
}


bool
ServiceManager::UnloadDriver()
{
    SERVICE_STATUS ServiceStatus = {0};

    dbg("Stopping service '%S'\n", CFB_BROKER_DRIVER_SERVICE_NAME);

    if ( !::ControlService(m_hService.get(), SERVICE_CONTROL_STOP, &ServiceStatus) )
    {
        CFB::Log::perror("ControlService()");
        return false;
    }

    dbg("Service '%S' stopped", CFB_BROKER_DRIVER_SERVICE_NAME);

    if ( !::DeleteService(m_hService.get()) )
    {
        CFB::Log::perror("DeleteService()");
        return false;
    }

    return true;
}


bool
ServiceManager::InstallBackgroundService()
{
    //
    // [1] Create the Win32 service
    //
    auto BrokerPath = Globals.Path();

    {
        const std::wstring BinaryPathName = L"\"" + BrokerPath.wstring() + L" service\"";

        wil::unique_handle hService(::CreateServiceW(
            m_hSCManager.get(),
            CFB_BROKER_WIN32_SERVICE_NAME,
            CFB_BROKER_WIN32_SERVICE_DESCRIPTION,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            BinaryPathName.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr));
        if ( !hService )
        {
            CFB::Log::perror("CreateServiceW()");
            return false;
        }
    }

    return true;
}


bool
ServiceManager::RunAsBackgroundService()
{
    //
    // [1] Declare the win32 service mode to the global context
    //
    {
        try
        {
            m_BackgroundService = std::make_shared<Win32Service>();
        }
        catch ( ... )
        {
            return false;
        }
    }

    //
    // [2] Start the service through the SCM
    //
    {
        auto lpswServiceName = (LPWSTR)CFB_BROKER_WIN32_SERVICE_NAME;

        SERVICE_TABLE_ENTRYW ServiceTable[] = {
            {lpswServiceName, (LPSERVICE_MAIN_FUNCTIONW)ServiceMain},
            {nullptr, nullptr}};

        if ( !::StartServiceCtrlDispatcherW(ServiceTable) )
        {
            CFB::Log::perror("StartServiceCtrlDispatcherW()");
            return false;
        }
    }

    return true;
}


void
ServiceManager::Run()
{
    //
    // Notify other thread the driver service is ready
    //
    NotifyNewState(CFB::Broker::State::ServiceManagerReady);

    //
    // Simply wait for the other managers to be done
    //
    WaitForState(CFB::Broker::State::IrpManagerDone);
}


std::string const
ServiceManager::Name()
{
    return "ServiceManager";
}


///////////////////////////////////////////////////////////////
///
/// Methods related to the Win32 service
///

Win32Service::Win32Service() : m_State(ServiceState::Uninitialized), m_StatusHandle(), m_StatusCheckPoint(0)
{
    dbg("Initializing background service");

    m_ServiceStateChangedEvent = ::CreateEventW(nullptr, true, false, nullptr);
    if ( !m_ServiceStateChangedEvent )
    {
        CFB::Log::perror("CreateEvent()");
        throw std::runtime_error("Win32Service() failed");
    }

    //
    // Register the controller, and get the service status handle
    //
    {
        SERVICE_STATUS_HANDLE hServiceStatus = ::RegisterServiceCtrlHandlerW(L"", ServiceCtrlHandler);
        if ( !hServiceStatus )
        {
            CFB::Log::perror("RegisterServiceCtrlHandler()");
            throw std::runtime_error("Win32Service() failed");
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

        if ( ReportServiceStatus(&ServiceStatus) == false )
        {
            throw std::runtime_error("Win32Service() failed");
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

        if ( ReportServiceStatus(&ServiceStatus) == false )
        {
            throw std::runtime_error("Win32Service() failed");
        }
    }
}


Win32Service::~Win32Service()
{
    dbg("Terminating background service");
}


void
Win32Service::RunForever()
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
            if ( m_State == ServiceState::ShuttingDown )
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

        if ( ReportServiceStatus(&ServiceStatus) == false )
        {
            CFB::Log::perror("SetServiceStatus() failed");
        }
    }
}


bool
Win32Service::Notify(ServiceState NewState)
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
Win32Service::Stop()
{
    SERVICE_STATUS ServiceStatus     = {0};
    ServiceStatus.dwControlsAccepted = 0;
    ServiceStatus.dwCurrentState     = SERVICE_STOP_PENDING;
    ServiceStatus.dwWin32ExitCode    = 0;
    ServiceStatus.dwCheckPoint       = 4;

    if ( ReportServiceStatus(&ServiceStatus) == false )
    {
        CFB::Log::perror("SetServiceStatus()");
        return false;
    }

    return true;
}


bool
Win32Service::ReportServiceStatus(LPSERVICE_STATUS lpServiceStatus)
{
    if ( !::SetServiceStatus(m_StatusHandle, lpServiceStatus) )
    {
        CFB::Log::perror("SetServiceStatus()");
        return false;
    }

    ServiceState NewState;

    switch ( lpServiceStatus->dwCurrentState )
    {
    case SERVICE_START_PENDING:
        NewState = ServiceState::Initialized;
        break;

    case SERVICE_RUNNING:
        NewState = ServiceState::Running;
        break;

    case SERVICE_STOP_PENDING:
        NewState = ServiceState::ShuttingDown;
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
Win32Service::InitializeRoutine()
{
    //
    // Expect the state to be `Uninitialized`, otherwise just bail
    //
    if ( m_State != ServiceState::Uninitialized )
    {
        warn("Invalid state");
        return false;
    }


    return true;
}


static VOID
ServiceMain(DWORD argc, LPWSTR* argv)
{
    if ( Globals.ServiceManager() == nullptr )
    {
        return;
    }

    auto svc = Globals.ServiceManager()->BackgroundService();
    if ( svc == nullptr )
    {
        warn("Trying to execute the service handler when no service exists...");
        return;
    }

    svc->RunForever();

    dbg("%S background service ready, starting thread...", CFB_BROKER_WIN32_SERVICE_NAME);
    return;
}


static VOID
ServiceCtrlHandler(const DWORD dwCtrlCode)
{
    if ( Globals.ServiceManager() == nullptr )
    {
        return;
    }

    auto svc = Globals.ServiceManager()->BackgroundService();
    if ( svc == nullptr )
    {
        warn("Trying to execute the service handler when no service exists...");
        return;
    }

    switch ( dwCtrlCode )
    {
    case SERVICE_CONTROL_STOP:
        svc->Stop();
        break;

    case SERVICE_CONTROL_CONTINUE:
        warn("Unhandled control code: SERVICE_CONTROL_CONTINUE");
        break;

    case SERVICE_CONTROL_INTERROGATE:
        warn("Unhandled control code: SERVICE_CONTROL_INTERROGATE");
        break;

    case SERVICE_CONTROL_PAUSE:
        warn("Unhandled control code: SERVICE_CONTROL_PAUSE");
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        warn("Unhandled control code: SERVICE_CONTROL_SHUTDOWN");
        break;

    default:
        break;
    }

    return;
}

} // namespace CFB::Broker
