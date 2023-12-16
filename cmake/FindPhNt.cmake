include(FetchContent)

FetchContent_Declare(
    phnt
    GIT_REPOSITORY https://github.com/winsiderss/phnt.git
    GIT_TAG 7c1adb8a7391939dfd684f27a37e31f18d303944
)
FetchContent_MakeAvailable(phnt)
message(STATUS "Using PhNt in '${phnt_SOURCE_DIR}'")
add_library(Deps::PHNT ALIAS phnt)
