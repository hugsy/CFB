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

#include "Common.hpp"
// clang-format on

int
main(int argc, const char** argv)
{
    argparse::ArgumentParser program("Broker");

    const std::vector<std::string> valid_actions = {"run", "install", "uninstall"};
    program.add_argument("--action")
        .default_value(valid_actions.front())
        .action(
            [&valid_actions](const std::string& value)
            {
                if ( std::find(valid_actions.cbegin(), valid_actions.cend(), value) != valid_actions.cend() )
                {
                    return value;
                }
                return valid_actions.front();
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

    return 0;
}
