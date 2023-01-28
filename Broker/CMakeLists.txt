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
    ${BROKER_HEADER_DIR}/Error.hpp
    ${BROKER_HEADER_DIR}/Broker.hpp
    ${BROKER_HEADER_DIR}/Native.hpp
    ${BROKER_HEADER_DIR}/Context.hpp
    ${BROKER_HEADER_DIR}/States.hpp
    ${BROKER_HEADER_DIR}/BrokerUtils.hpp
    ${BROKER_HEADER_DIR}/ManagerBase.hpp
    ${BROKER_HEADER_DIR}/ServiceManager.hpp
    ${BROKER_HEADER_DIR}/IrpManager.hpp
    ${BROKER_HEADER_DIR}/ConnectorManager.hpp
    ${BROKER_HEADER_DIR}/DriverManager.hpp

    ${BROKER_HEADER_DIR}/Connectors/Base.hpp
    ${BROKER_HEADER_DIR}/Connectors/Dummy.hpp
)

set(
    SOURCE_FILES

    ${BROKER_SOURCE_DIR}/Main.cpp
    ${BROKER_SOURCE_DIR}/Context.cpp
    ${BROKER_SOURCE_DIR}/BrokerUtils.cpp
    ${BROKER_SOURCE_DIR}/ManagerBase.cpp
    ${BROKER_SOURCE_DIR}/ServiceManager.cpp
    ${BROKER_SOURCE_DIR}/IrpManager.cpp
    ${BROKER_SOURCE_DIR}/ConnectorManager.cpp
    ${BROKER_SOURCE_DIR}/DriverManager.cpp
    ${BROKER_SOURCE_DIR}/DriverManagerListener.cpp

    ${BROKER_SOURCE_DIR}/Connectors/Dummy.cpp
)

#
# Create the broker rc file
#
cmake_path(SET CFB_BROKER_DRIVER_PATH "$ENV{TEMP}/IrpMonitor.sys")
cmake_path(SET CFB_BROKER_ICON_PATH "${CFB_ROOT_DIR}/Assets/img/logo/logo.ico")
configure_file(${BROKER_RC_FILE}.in ${BROKER_RC_FILE} NEWLINE_STYLE WIN32 ESCAPE_QUOTES)

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
    ${CFB_DEPENDENCIES}/json
)

target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    ${BROKER_HEADER_DIR}/pch.hpp
)

#
# Compilation directives
#
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>

    PRIVATE
    $<IF:$<CONFIG:Debug>,/WX /Gm- /permissive-,/WX /permissive>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/fsanitize=address>>
)

#
# Linking directives
#
target_link_options(
    ${PROJECT_NAME}
    PUBLIC
    /SUBSYSTEM:Console

    PRIVATE
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/InferAsanLibs>>
)

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
    LINK_FLAGS " /level='requireAdministrator' /uiAccess='false' "
)

#
# Install directives
#
install(TARGETS ${PROJECT_NAME} DESTINATION Tools)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION Tools OPTIONAL)

#
# Custom command directives
#
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND
    rc /nologo /fo ${BROKER_RES_FILE} /r ${BROKER_RC_FILE}
    COMMENT
    " Generating the .res file "
)
