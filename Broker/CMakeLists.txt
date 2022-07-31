project(Broker LANGUAGES CXX VERSION 0.2.0)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(BROKER_SOURCE_DIR ${CFB_ROOT_DIR}/Broker)
set(BROKER_HEADER_DIR ${BROKER_SOURCE_DIR})

set(BROKER_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "BROKER_VERSION_MAJOR")
set(BROKER_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "BROKER_VERSION_MINOR")
set(BROKER_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "BROKER_VERSION_PATCH")

set(BROKER_RC_FILE ${BROKER_SOURCE_DIR}/Broker.rc)
set(BROKER_RES_FILE ${BROKER_SOURCE_DIR}/Broker.res)

#
# Source files
#
set(
    HEADER_FILES

    ${BROKER_HEADER_DIR}/Resource.h
    ${BROKER_HEADER_DIR}/Broker.hpp
    ${BROKER_HEADER_DIR}/Native.hpp
    ${BROKER_HEADER_DIR}/Context.hpp
    ${BROKER_HEADER_DIR}/States.hpp
    ${BROKER_HEADER_DIR}/BrokerUtils.hpp
    ${BROKER_HEADER_DIR}/ManagerBase.hpp

    ${BROKER_HEADER_DIR}/ServiceManager.hpp

    # ${BROKER_HEADER_DIR}/ConnectorManager.hpp
)

set(
    SOURCE_FILES

    ${BROKER_SOURCE_DIR}/Main.cpp
    ${BROKER_SOURCE_DIR}/Context.cpp
    ${BROKER_SOURCE_DIR}/BrokerUtils.cpp
    ${BROKER_SOURCE_DIR}/ManagerBase.cpp

    ${BROKER_SOURCE_DIR}/ServiceManager.cpp

    # ${BROKER_SOURCE_DIR}/ConnectorManager.cpp
)

#
# Create the broker rc file
#
set(CFB_BROKER_DRIVER_PATH ${CFB_DRIVER_OUTPUT_PATH})
set(CFB_BROKER_ICON_PATH ${CFB_ROOT_DIR}/Assets/img/logo/logo.ico)
configure_file(${BROKER_RC_FILE}.in ${BROKER_RC_FILE} NEWLINE_STYLE WIN32)

#
# Create the executable target
#
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
add_dependencies(${PROJECT_NAME} IrpMonitor)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${BROKER_HEADER_DIR}
    CommonLibUser
    ${CFB_DEPENDENCIES}/wil/include
    ${CFB_DEPENDENCIES}/argparse
    ${CFB_DEPENDENCIES}/phnt
    ${CFB_DEPENDENCIES}/httplib
)

target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    ${BROKER_HEADER_DIR}/pch.hpp
)

if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

#
# Compilation directives
#
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

#
# Linking directives
#
target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:Console)
target_link_libraries(${PROJECT_NAME}
    ${BROKER_RES_FILE}
    ws2_32.lib
    Userenv.lib
    Rpcrt4.lib
    kernel32.lib
    ntdll.lib
    Advapi32.lib
    CommonLibUser
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'"
)

#
# Install directives
#
install(TARGETS ${PROJECT_NAME} DESTINATION Broker)

#
# Custom command directives
#
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND
    rc /nologo /fo ${BROKER_RES_FILE} /r ${BROKER_RC_FILE}
    COMMENT
    "Generating the .res file"
)
