project(IrpMonitor
    LANGUAGES CXX
    VERSION 0.2.0
    DESCRIPTION "Driver part of CFB"
)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(DRIVER_SOURCE_DIR ${CFB_ROOT_DIR}/Driver)
set(DRIVER_HEADER_DIR ${DRIVER_SOURCE_DIR})

set(DRIVER_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "DRIVER_VERSION_MAJOR")
set(DRIVER_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "DRIVER_VERSION_MINOR")
set(DRIVER_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "DRIVER_VERSION_PATCH")

set(DRIVER_RC_FILE ${DRIVER_SOURCE_DIR}/Driver.rc)
set(DRIVER_RES_FILE ${DRIVER_SOURCE_DIR}/Driver.res)
configure_file(${DRIVER_RC_FILE}.in ${DRIVER_RC_FILE} NEWLINE_STYLE WIN32 ESCAPE_QUOTES)

set(
    DRIVER_HEADER_FILES

    ${DRIVER_HEADER_DIR}/Context.hpp
    ${DRIVER_HEADER_DIR}/Native.hpp
    ${DRIVER_HEADER_DIR}/DriverUtils.hpp
    ${DRIVER_HEADER_DIR}/HookedDriverManager.hpp
    ${DRIVER_HEADER_DIR}/HookedDriver.hpp
    ${DRIVER_HEADER_DIR}/Callbacks.hpp
    ${DRIVER_HEADER_DIR}/CapturedIrpManager.hpp
    ${DRIVER_HEADER_DIR}/CapturedIrp.hpp
)

set(
    DRIVER_SOURCE_FILES

    ${DRIVER_SOURCE_DIR}/Entry.cpp
    ${DRIVER_SOURCE_DIR}/DriverUtils.cpp
    ${DRIVER_SOURCE_DIR}/HookedDriverManager.cpp
    ${DRIVER_SOURCE_DIR}/HookedDriver.cpp
    ${DRIVER_SOURCE_DIR}/Callbacks.cpp
    ${DRIVER_SOURCE_DIR}/CapturedIrpManager.cpp
    ${DRIVER_SOURCE_DIR}/CapturedIrp.cpp
)

wdk_add_driver(
    ${PROJECT_NAME}
    KMDF
    1.15

    ${DRIVER_HEADER_FILES}
    ${DRIVER_SOURCE_FILES}
)

add_executable(CFB::Kernel::Driver ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC CFB::Kernel::CommonLib)
target_compile_definitions(${PROJECT_NAME} PRIVATE CFB_KERNEL_DRIVER=1)
target_link_options(${PROJECT_NAME} PUBLIC /integritycheck)
target_link_libraries(${PROJECT_NAME} CFB::Kernel::CommonLib ${DRIVER_RES_FILE})

set(CFB_CERT_CN "${CFB_COMPANY_NAME}")
set(CFB_CERT_STORE "PrivateCertStore")
set(CFB_DRIVER_INF_FILE "${DRIVER_SOURCE_DIR}/${PROJECT_NAME}.inf")

#
# Custom command directives
#
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND
    rc /nologo /fo ${DRIVER_RES_FILE} /r ${DRIVER_RC_FILE}
    COMMENT
    "Generating the .res file"
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    makecert.exe -r -pe -ss ${CFB_CERT_STORE} -n CN="${CFB_CERT_CN}" -eku 1.3.6.1.5.5.7.3.3 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/BlahCatTest.cer"
    COMMAND
    signtool.exe sign /v /a /fd SHA256 /s ${CFB_CERT_STORE} /n "${CFB_CERT_CN}" $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND
    certmgr.exe -del -c -n "${CFB_CERT_CN}" -s -r currentUser ${CFB_CERT_STORE}
    COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "$ENV{TEMP}"
    COMMENT
    "Sign driver, copy to tempdir"
)

#
# Install directives
#
install(TARGETS ${PROJECT_NAME} DESTINATION Driver)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION Driver OPTIONAL)
install(FILES ${CFB_DRIVER_INF_FILE} DESTINATION Driver OPTIONAL)
