project(IrpMonitor LANGUAGES CXX VERSION 0.2.0)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(DRIVER_SOURCE_DIR ${CFB_ROOT_DIR}/Driver)
set(DRIVER_HEADER_DIR ${DRIVER_SOURCE_DIR})

set(
    DRIVER_HEADER_FILES

    ${DRIVER_HEADER_DIR}/Entry.hpp
    ${DRIVER_HEADER_DIR}/DriverUtils.hpp
    ${DRIVER_HEADER_DIR}/IoDriver.hpp
)

set(
    DRIVER_SOURCE_FILES

    ${DRIVER_SOURCE_DIR}/Entry.cpp
    ${DRIVER_SOURCE_DIR}/DriverUtils.cpp

    ${DRIVER_SOURCE_DIR}/IoAddDriver.cpp
    ${DRIVER_SOURCE_DIR}/IoRemoveDriver.cpp
)

wdk_add_driver(
    ${PROJECT_NAME}
    KMDF
    1.15

    ${DRIVER_HEADER_FILES}
    ${DRIVER_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC CommonLibDriver ${DRIVER_HEADER_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE CFB_KERNEL_DRIVER=1)
target_link_options(${PROJECT_NAME} PUBLIC /integritycheck)
target_link_libraries(${PROJECT_NAME} CommonLibDriver)

if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif(DEBUG)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    makecert -r -pe -ss PrivateCertStore -n CN="BlahCat Corp" -eku 1.3.6.1.5.5.7.3.3 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/BlahCatTest.cer"
    COMMAND
    signtool sign /v /a /fd SHA256 /s PrivateCertStore /n "BlahCat Corp" $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT
    "Test-signing driver"
)

if(CFB_BUILD_TOOLS)
    add_subdirectory(Client)
endif(CFB_BUILD_TOOLS)
