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

#
# Check the dependencies
#
message(STATUS "Locating Windows Driver Kit")
find_package(WDK REQUIRED)
find_path(PHNT_INCLUDE_DIRS "ntd3dkmt.h")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

if(CFB_BUILD_GUI)
    list(APPEND VCPKG_MANIFEST_FEATURES "gui")
    find_package(imgui CONFIG REQUIRED)
endif(CFB_BUILD_GUI)

if(CFB_BUILD_TOOLS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tools")
    find_package(argparse CONFIG REQUIRED)
    find_package(wil CONFIG REQUIRED)
endif(CFB_BUILD_TOOLS)

if(CFB_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
    find_package(Catch2 CONFIG REQUIRED)
    find_package(trompeloeil CONFIG REQUIRED)
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

if(CFB_BUILD_TESTS)
    include(CTest)
    add_subdirectory(${CFB_ROOT_DIR}/Common/Tests)
endif(CFB_BUILD_TESTS)

if(CFB_BUILD_BROKER)
    add_subdirectory(${CFB_ROOT_DIR}/Driver)
    add_subdirectory(${CFB_ROOT_DIR}/Broker)

    if(CFB_BUILD_TESTS)
        add_subdirectory(${CFB_ROOT_DIR}/Broker/Tests)
    endif(CFB_BUILD_TESTS)

    if(CFB_BUILD_TOOLS)
        add_subdirectory(${CFB_ROOT_DIR}/Driver/Client)
    endif(CFB_BUILD_TOOLS)
endif(CFB_BUILD_BROKER)

if(CFB_BUILD_GUI)
    add_subdirectory(${CFB_ROOT_DIR}/GUI)

    if(CFB_BUILD_TESTS)
        add_subdirectory(${CFB_ROOT_DIR}/GUI/Tests)
    endif(CFB_BUILD_TESTS)
endif(CFB_BUILD_GUI)
