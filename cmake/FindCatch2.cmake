include(FetchContent)

FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.zip
    URL_HASH MD5=c426e77d4ee0055410bc930182959ae5
)

FetchContent_MakeAvailable(Catch2)
message(STATUS "Using Catch2 in '${Catch2_SOURCE_DIR}'")
add_library(Deps::Catch2 ALIAS Catch2)
