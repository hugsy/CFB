set(DRIVER_CLIENT_SOURCE_DIR ${CFB_ROOT_DIR}/Driver/Client)

set(
    SOURCE_FILES

    ${DRIVER_CLIENT_SOURCE_DIR}/Main.cpp
)

add_executable(DriverClient WIN32)
add_executable(CFB::Tools::DriverClient ALIAS DriverClient)
target_sources(DriverClient PUBLIC ${SOURCE_FILES})
target_compile_features(DriverClient PRIVATE cxx_std_20)

add_dependencies(
    DriverClient
    CFB::Kernel::Driver
    CFB::User::CommonLib
)

if(DEBUG)
    target_compile_definitions(DriverClient PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

target_include_directories(
    DriverClient
    PRIVATE
)

target_link_libraries(DriverClient
    PRIVATE
    CFB::User::CommonLib
    argparse::argparse
    WIL::WIL
)

target_compile_options(DriverClient
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>

    PRIVATE
    $<IF:$<CONFIG:Debug>,/WX /Gm- /permissive-,/WX /permissive>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/fsanitize=address>>
)

target_link_options(DriverClient
    PUBLIC
    /SUBSYSTEM:Console

    PRIVATE
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/InferAsanLibs>>
)

install(TARGETS DriverClient DESTINATION Tools)
install(FILES $<TARGET_PDB_FILE:DriverClient> DESTINATION Tools OPTIONAL)
