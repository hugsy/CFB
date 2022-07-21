set(DRIVER_CLIENT_SOURCE_DIR ${DRIVER_SOURCE_DIR}/Client)

set(
    SOURCE_FILES

    ${DRIVER_CLIENT_SOURCE_DIR}/Main.cpp
)

add_executable(DriverClient
    WIN32
    ${SOURCE_FILES}
)

add_dependencies(DriverClient ${PROJECT_NAME})

if(DEBUG)
    target_compile_definitions(DriverClient PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

target_include_directories(DriverClient PUBLIC CommonLibUser ${CFB_DEPENDENCIES}/wil/include ${CFB_DEPENDENCIES}/argparse)
target_link_options(DriverClient PUBLIC /SUBSYSTEM:Console)
target_link_libraries(DriverClient CommonLibUser)
