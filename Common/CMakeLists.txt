message(STATUS "Configuring 'CommonLib'")

set(COMMON_SOURCE_DIR ${CFB_ROOT_DIR}/Common)
set(COMMON_HEADER_DIR ${COMMON_SOURCE_DIR})

set(
    COMMON_HEADER_FILES

    ${COMMON_HEADER_DIR}/CompileInfo.hpp

    ${COMMON_HEADER_DIR}/Common.hpp
    ${COMMON_HEADER_DIR}/Log.hpp
    ${COMMON_HEADER_DIR}/Messages.hpp
    ${COMMON_HEADER_DIR}/Utils.hpp
    ${COMMON_HEADER_DIR}/IoctlCodes.hpp
)

set(
    COMMON_SOURCE_FILES

    ${COMMON_SOURCE_DIR}/Common.cpp
    ${COMMON_SOURCE_DIR}/Log.cpp
    ${COMMON_SOURCE_DIR}/Utils.cpp
)

#
# Create the compilation information file
#
configure_file(
    ${COMMON_HEADER_DIR}/CompileInfo.hpp.in
    ${COMMON_HEADER_DIR}/CompileInfo.hpp
    NEWLINE_STYLE WIN32
)

#
# Build CommonLib for driver
#
wdk_add_library(CommonLibDriver
    STATIC
    KMDF
    1.15

    ${COMMON_SOURCE_FILES}
    ${COMMON_HEADER_FILES}
)

target_include_directories(CommonLibDriver INTERFACE ${COMMON_SOURCE_DIR})
target_compile_definitions(CommonLibDriver PRIVATE CFB_KERNEL_DRIVER=1)

if(DEBUG)
    target_compile_definitions(CommonLibDriver PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

#
# Build CommonLib for user
#
add_library(CommonLibUser
    STATIC

    ${COMMON_SOURCE_FILES}
    ${COMMON_HEADER_FILES}
)

target_include_directories(CommonLibUser INTERFACE ${COMMON_SOURCE_DIR} PUBLIC ${CFB_DEPENDENCIES}/phnt)

if(DEBUG)
    target_compile_definitions(CommonLibUser PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

target_link_libraries(CommonLibUser PRIVATE kernel32.lib ntdll.lib)
