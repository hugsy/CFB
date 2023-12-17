include(FetchContent)
set(FAST_BUILD OFF)
set(WIL_BUILD_PACKAGING OFF)
set(WIL_BUILD_TESTS OFF)

FetchContent_Declare(
    WIL
    URL https://github.com/microsoft/wil/archive/refs/tags/v1.0.231028.1.zip
    URL_HASH MD5=48f04bde1b5d745ee2f6dedc9040fba7
)
FetchContent_MakeAvailable(WIL)
message(STATUS "Using WIL in '${WIL_SOURCE_DIR}'")

add_library(Deps_WIL INTERFACE EXCLUDE_FROM_ALL)
add_library(Deps::WIL ALIAS Deps_WIL)
target_include_directories(Deps_WIL INTERFACE ${WIL_SOURCE_DIR}/include)
