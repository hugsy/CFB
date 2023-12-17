include(FetchContent)

FetchContent_Declare(
    argparse
    URL https://github.com/p-ranav/argparse/archive/refs/tags/v3.0.zip
    URL_HASH MD5=a44c0401238e87239e31652b72fded20
)
FetchContent_MakeAvailable(argparse)
message(STATUS "Using ArgParse in '${argparse_SOURCE_DIR}'")

add_library(Deps_Argparse INTERFACE EXCLUDE_FROM_ALL)
target_compile_features(Deps_Argparse INTERFACE cxx_std_20)
target_include_directories(Deps_Argparse INTERFACE ${argparse_SOURCE_DIR}/include)
add_library(Deps::Argparse ALIAS Deps_Argparse)
