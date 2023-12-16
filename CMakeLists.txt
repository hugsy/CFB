cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(
    CFB
    LANGUAGES CXX
    VERSION 0.2.0
    DESCRIPTION "Canadian Furious Beaver - IRP interceptor and replayer"
    HOMEPAGE_URL https://github.com/hugsy/cfb
)

set(PROJECT_AUTHOR hugsy)
set(PROJECT_LICENSE MIT)

set(CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(CFB_BUILD_GUI "Build GUI" ON)
option(CFB_BUILD_TOOLS "Build Tools" OFF)
option(CFB_BUILD_TESTS "Build Tests" OFF)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

#
# Check the dependencies
#
message(STATUS "Locating Windows Driver Kit")
find_package(WDK REQUIRED)
find_package(PhNt REQUIRED)
find_package(WIL REQUIRED)
find_package(Json REQUIRED)
find_package(argparse REQUIRED)

if(CFB_BUILD_GUI)
    find_package(ImGUI REQUIRED)
endif(CFB_BUILD_GUI)

if(CFB_BUILD_TESTS)
    find_package(Catch2 REQUIRED)
endif(CFB_BUILD_TESTS)

set(CFB_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CFB_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "CFB_VERSION_MAJOR")
set(CFB_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "CFB_VERSION_MINOR")
set(CFB_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "CFB_VERSION_PATCH")
set(CFB_COMPANY_NAME "BlahCat Corp." CACHE INTERNAL "CFB_COMPANY_NAME")
string(TIMESTAMP CFB_CURRENT_YEAR "%Y")

#
# Build
#
add_subdirectory(${CFB_ROOT_DIR}/Common)
add_subdirectory(${CFB_ROOT_DIR}/Driver)
add_subdirectory(${CFB_ROOT_DIR}/Broker)

if(CFB_BUILD_GUI)
    add_subdirectory(${CFB_ROOT_DIR}/GUI)
endif(CFB_BUILD_GUI)

if(CFB_BUILD_TOOLS)
    add_subdirectory(${CFB_ROOT_DIR}/Driver/Client)
endif(CFB_BUILD_TOOLS)

if(CFB_BUILD_TESTS)
    include(CTest)
    add_subdirectory(${CFB_ROOT_DIR}/Common/Tests)
    add_subdirectory(${CFB_ROOT_DIR}/Broker/Tests)
    add_subdirectory(${CFB_ROOT_DIR}/GUI/Tests)
endif(CFB_BUILD_TESTS)
