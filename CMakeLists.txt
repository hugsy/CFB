cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(
    CFB
    LANGUAGES CXX
    VERSION 0.2.0
    DESCRIPTION "Canadian Furious Beaver - IRP hooker, interceptor and replayer"
    HOMEPAGE_URL https://github.com/hugsy/cfb
)

set(PROJECT_AUTHOR hugsy)
set(PROJECT_LICENSE MIT)

set(CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(DEBUG "Enable Debug" ON)
option(CFB_BUILD_TOOLS "Build Tools" OFF)

message(STATUS "Locating Windows Driver Kit")
find_package(WDK REQUIRED)

set(CFB_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CFB_DEPENDENCIES ${CFB_ROOT_DIR}/External)

#
# Build WIL (External)
#
# set(CPPWINRT_VERSION "2.0.220608.4")
set(WIL_BUILD_PACKAGING OFF)
set(WIL_BUILD_TESTS OFF)

add_subdirectory(${CFB_DEPENDENCIES}/wil)

#
# Build the common library
#
add_subdirectory(${CFB_ROOT_DIR}/Common)

#
# Build Driver
#
add_subdirectory(${CFB_ROOT_DIR}/Driver)

#
# Build Broker
#
# add_subdirectory(Broker)

#
# Build Driver
#
# add_subdirectory(GUI)