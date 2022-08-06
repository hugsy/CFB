#define NOMINMAX
#undef _UNICODE
#undef UNICODE

// clang-format off
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

#include <argparse.hpp>
#include <wil/resource.h>

#include "Broker.hpp"
#include "Common.hpp"
#include "BrokerUtils.hpp"
#include "Context.hpp"
#include "Log.hpp"
// clang-format on

using namespace ::std::literals;

struct GlobalContext Globals;


int
main(int argc, const char** argv)
{
    argparse::ArgumentParser program("Broker");

    const std::vector<std::string> valid_modes = {"run-standalone", "install-service", "run-as-service"};
    program.add_argument("mode")
        .remaining()
        .default_value(valid_modes.front())
        .action(
            [&valid_modes](const std::string& value)
            {
                if ( std::find(valid_modes.cbegin(), valid_modes.cend(), value) != valid_modes.cend() )
                {
                    return value;
                }
                return valid_modes.front();
            });

    try
    {
        program.parse_args(argc, argv);
    }
    catch ( const std::runtime_error& err )
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if ( Failed(CFB::Broker::Utils::AcquirePrivileges({L"SeDebugPrivilege", L"SeLoadDriverPrivilege"})) )
    {
        err("Cannot required privileged, cannot continue");
        std::exit(-2);
    }


    auto const& mode = program.get<std::string>("mode");

    if ( mode == "run-standalone" )
    {
        info("Running in standalone...");
        std::cin.get();

        info("Finishing...");
        Globals.Stop();
    }

    else if ( mode == "install-service" )
    {
        Globals.ServiceManager()->InstallBackgroundService();
    }

    else if ( mode == "run-as-service" )
    {
        Globals.ServiceManager()->RunAsBackgroundService();
    }

    return 0;
}
