set(DRIVER_CLIENT_SOURCE_DIR ${CFB_ROOT_DIR}/Driver/Client)

set(
    SOURCE_FILES

    ${DRIVER_CLIENT_SOURCE_DIR}/Main.cpp
)

add_executable(DriverClient WIN32)
target_sources(DriverClient PUBLIC ${SOURCE_FILES})

add_dependencies(DriverClient CFB::Kernel::Driver)

if(DEBUG)
    target_compile_definitions(DriverClient PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

target_include_directories(
    DriverClient
    PRIVATE
    CFB::User::CommonLib
    argparse::argparse
    WIL::WIL
)
target_link_options(DriverClient PUBLIC /SUBSYSTEM:Console)
target_link_libraries(
    DriverClient
    PRIVATE
    CFB::User::CommonLib
    argparse::argparse
    WIL::WIL
)

install(TARGETS DriverClient DESTINATION Tools)
install(FILES $<TARGET_PDB_FILE:DriverClient> DESTINATION Tools OPTIONAL)
