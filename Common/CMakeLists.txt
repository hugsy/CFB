message(STATUS "Configuring 'CommonLib'")

set(COMMON_SOURCE_DIR ${CFB_ROOT_DIR}/Common/Source)
set(COMMON_HEADER_DIR ${CFB_ROOT_DIR}/Common/Headers)

set(
    COMMON_HEADER_FILES

    ${COMMON_HEADER_DIR}/CompileInfo.hpp

    ${COMMON_HEADER_DIR}/Common.hpp
    ${COMMON_HEADER_DIR}/Comms.hpp
    ${COMMON_HEADER_DIR}/IoctlCodes.hpp
    ${COMMON_HEADER_DIR}/Log.hpp
    ${COMMON_HEADER_DIR}/Messages.hpp
    ${COMMON_HEADER_DIR}/Utils.hpp
)

set(
    COMMON_SOURCE_FILES

    ${COMMON_SOURCE_DIR}/Comms.cpp
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
set(
    COMMON_DRIVER_SOURCE_FILES

    ${COMMON_SOURCE_FILES}
)

set(
    COMMON_DRIVER_HEADER_FILES

    ${COMMON_HEADER_FILES}
)

wdk_add_library(CommonLibDriver
    STATIC
    KMDF
    1.15

    ${COMMON_DRIVER_SOURCE_FILES}
    ${COMMON_DRIVER_HEADER_FILES}
)

add_library(CFB::Kernel::CommonLib ALIAS CommonLibDriver)

target_include_directories(CommonLibDriver PUBLIC ${COMMON_HEADER_DIR})
target_compile_definitions(CommonLibDriver PRIVATE CFB_KERNEL_DRIVER=1)

#
# Build CommonLib for user
#
set(
    COMMON_USER_SOURCE_FILES

    ${COMMON_SOURCE_FILES}
    ${COMMON_SOURCE_DIR}/Messages.cpp
)

set(
    COMMON_USER_HEADER_FILES

    ${COMMON_HEADER_FILES}
    ${COMMON_HEADER_DIR}/Messages.hpp
)

add_library(CommonLibUser
    STATIC

    ${COMMON_USER_SOURCE_FILES}
    ${COMMON_USER_HEADER_FILES}
)

add_library(CFB::User::CommonLib ALIAS CommonLibUser)

target_include_directories(CommonLibUser
    PUBLIC
    ${COMMON_HEADER_DIR}

    $<TARGET_PROPERTY:Deps::JSON,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_options(CommonLibUser
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/fsanitize=address>>

    PRIVATE
    $<IF:$<CONFIG:Debug>,/WX /Gm- /permissive-,/WX /permissive>
)

target_compile_definitions(CommonLibUser
    PUBLIC
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:_DISABLE_VECTOR_ANNOTATION>>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:_DISABLE_STRING_ANNOTATION>>

    PRIVATE
)

target_link_options(CommonLibUser
    PUBLIC
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/InferAsanLibs>>

    PRIVATE
)

target_link_libraries(CommonLibUser PRIVATE kernel32.lib ntdll.lib)
