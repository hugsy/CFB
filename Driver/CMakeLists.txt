project(IrpMonitor LANGUAGES CXX VERSION 0.2.0)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(DRIVER_SOURCE_DIR ${CFB_ROOT_DIR}/Driver)
set(DRIVER_HEADER_DIR ${DRIVER_SOURCE_DIR})

set(
    DRIVER_HEADER_FILES

    ${DRIVER_HEADER_DIR}/Context.hpp
    ${DRIVER_HEADER_DIR}/Native.hpp
    ${DRIVER_HEADER_DIR}/DriverUtils.hpp
    ${DRIVER_HEADER_DIR}/HookedDriverManager.hpp
    ${DRIVER_HEADER_DIR}/HookedDriver.hpp
    ${DRIVER_HEADER_DIR}/Callbacks.hpp
)

set(
    DRIVER_SOURCE_FILES

    ${DRIVER_SOURCE_DIR}/Entry.cpp
    ${DRIVER_SOURCE_DIR}/DriverUtils.cpp
    ${DRIVER_SOURCE_DIR}/HookedDriverManager.cpp
    ${DRIVER_SOURCE_DIR}/HookedDriver.cpp
    ${DRIVER_SOURCE_DIR}/Callbacks.cpp
)

wdk_add_driver(
    ${PROJECT_NAME}
    KMDF
    1.15

    ${DRIVER_HEADER_FILES}
    ${DRIVER_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC CommonLibDriver)
target_compile_definitions(${PROJECT_NAME} PRIVATE CFB_KERNEL_DRIVER=1)
target_link_options(${PROJECT_NAME} PUBLIC /integritycheck)
target_link_libraries(${PROJECT_NAME} CommonLibDriver)

if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif(DEBUG)

set(CFB_CERT_CN "BlahCat Corp")
set(CFB_CERT_STORE "PrivateCertStore")

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    makecert.exe -r -pe -ss ${CFB_CERT_STORE} -n CN="${CFB_CERT_CN}" -eku 1.3.6.1.5.5.7.3.3 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/BlahCatTest.cer"
    COMMAND
    signtool.exe sign /v /a /fd SHA256 /s ${CFB_CERT_STORE} /n "${CFB_CERT_CN}" $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND
    certmgr.exe -del -c -n "${CFB_CERT_CN}" -s -r currentUser ${CFB_CERT_STORE}
    COMMENT
    "Test-signing driver"
)

if(CFB_BUILD_TOOLS)
    add_subdirectory(Client)
endif(CFB_BUILD_TOOLS)
