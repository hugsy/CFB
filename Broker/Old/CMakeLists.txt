project(Broker LANGUAGES CXX VERSION 0.2.0)

message(STATUS "Configuring ${PROJECT_NAME}")

set(BROKER_SOURCE_DIR ${CFB_ROOT_DIR}/Broker)

set(
    BROKER_HEADER_FILES

    ${BROKER_SOURCE_DIR}/Main.hpp
    ${BROKER_SOURCE_DIR}/CfbException.hpp
    ${BROKER_SOURCE_DIR}/common.hpp
    ${BROKER_SOURCE_DIR}/driver.hpp
    ${BROKER_SOURCE_DIR}/FrontEndServer.hpp
    ${BROKER_SOURCE_DIR}/Irp.hpp
    ${BROKER_SOURCE_DIR}/nt.hpp
    ${BROKER_SOURCE_DIR}/PipeTransportManager.hpp
    ${BROKER_SOURCE_DIR}/resource.hpp
    ${BROKER_SOURCE_DIR}/SafeHandle.hpp
    ${BROKER_SOURCE_DIR}/ServerTransportManager.hpp
    ${BROKER_SOURCE_DIR}/ServiceManager.hpp
    ${BROKER_SOURCE_DIR}/Session.hpp
    ${BROKER_SOURCE_DIR}/Task.hpp
    ${BROKER_SOURCE_DIR}/TaskManager.hpp
    ${BROKER_SOURCE_DIR}/TcpSocketTransportManager.hpp
    ${BROKER_SOURCE_DIR}/Utils.hpp

    ${BROKER_ROOT_DIR}/External/json/json.hpp
)

set(
    BROKER_SOURCE_FILES

    ${BROKER_SOURCE_DIR}/Main.cpp

    ${BROKER_SOURCE_DIR}/CfbException.cpp
    ${BROKER_SOURCE_DIR}/Driver.cpp
    ${BROKER_SOURCE_DIR}/FrontEndServer.cpp
    ${BROKER_SOURCE_DIR}/Irp.cpp
    ${BROKER_SOURCE_DIR}/PipeTransportManager.cpp
    ${BROKER_SOURCE_DIR}/SafeHandle.cpp
    ${BROKER_SOURCE_DIR}/ServiceManager.cpp
    ${BROKER_SOURCE_DIR}/Session.cpp
    ${BROKER_SOURCE_DIR}/Task.cpp
    ${BROKER_SOURCE_DIR}/TaskManager.cpp
    ${BROKER_SOURCE_DIR}/TcpSocketTransportManager.cpp
    ${BROKER_SOURCE_DIR}/Utils.cpp
)

set(BROKER_RESOURCE_FILE ${BROKER_SOURCE_DIR}/${PROJECT_NAME}.rc)

add_executable(
    ${PROJECT_NAME}

    WIN32
    ${BROKER_SOURCE_FILES}
    ${BROKER_HEADER_FILES}
)

add_dependencies(${PROJECT_NAME} IrpMonitor)

target_include_directories(${PROJECT_NAME} PUBLIC CommonLib)
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
target_link_options(${PROJECT_NAME} PUBLIC /integritycheck)
target_link_libraries(${PROJECT_NAME} CommonLib)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND rc.exe ${BROKER_RESOURCE_FILE}
)
