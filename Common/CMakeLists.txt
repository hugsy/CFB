message(STATUS "Configuring 'CommonLib'")

set(COMMON_SOURCE_DIR ${CFB_ROOT_DIR}/Common)

set(
    COMMON_HEADER_FILES

    ${COMMON_SOURCE_DIR}/Common.hpp
    ${COMMON_SOURCE_DIR}/Log.hpp
    ${COMMON_SOURCE_DIR}/Utils.hpp
    ${COMMON_SOURCE_DIR}/IoctlCodes.hpp
)

set(
    COMMON_SOURCE_FILES

    ${COMMON_SOURCE_DIR}/Common.cpp
    ${COMMON_SOURCE_DIR}/Log.cpp
    ${COMMON_SOURCE_DIR}/Utils.cpp
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

target_include_directories(CommonLibUser INTERFACE ${COMMON_SOURCE_DIR})

if(DEBUG)
    target_compile_definitions(CommonLibDriver PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)
