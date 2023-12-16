include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip
    URL_HASH MD5=e2f46211f4cf5285412a63e8164d4ba6
)

FetchContent_MakeAvailable(nlohmann_json)
message(STATUS "Using Json in '${nlohmann_json_SOURCE_DIR}'")

add_library(nlohmann_json INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/single_include)
add_library(Deps::JSON ALIAS nlohmann_json)
