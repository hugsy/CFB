project(
    GUI
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "ImGui-based GUI for CFB"
    HOMEPAGE_URL https://github.com/hugsy/cfb
)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(GUI_SOURCE_DIR ${CFB_ROOT_DIR}/GUI)
set(GUI_HEADER_DIR ${GUI_SOURCE_DIR})

set(GUI_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "GUI_VERSION_MAJOR")
set(GUI_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "GUI_VERSION_MINOR")
set(GUI_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "GUI_VERSION_PATCH")

set(
    GUI_HEADER_FILES

    App.hpp
    Utils.hpp
    Helpers.hpp
    Network.hpp
)

set(
    GUI_SOURCE_FILES

    main.cpp
    App.cpp
    Utils.cpp
    Helpers.cpp
)

set(IMGUI_DIR ${CFB_DEPENDENCIES}/imgui)
set(IMGUI_BACKEND win32-dx12)

#
# Create the broker rc file
#
set(GUI_RC_FILE ${GUI_SOURCE_DIR}/GUI.rc)
set(GUI_RES_FILE ${GUI_SOURCE_DIR}/GUI.res)
cmake_path(SET CFB_GUI_ICON_PATH "${CFB_ROOT_DIR}/Assets/Images/logo/logo.ico")
configure_file(${GUI_RC_FILE}.in ${GUI_RC_FILE} NEWLINE_STYLE WIN32 ESCAPE_QUOTES)

#
# Build the GUI
#
add_executable(${PROJECT_NAME} ${GUI_SOURCE_FILES})

#
# Compilation directives
#
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    CFB::User::CommonLib

    PRIVATE
)

target_compile_definitions(${PROJECT_NAME} PUBLIC _UNICODE UNICODE ImTextureID=ImU64)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>

    PRIVATE
    $<IF:$<CONFIG:Debug>,/WX /Gm- /permissive-,/WX /permissive>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/fsanitize=address>>
)

#
# Linking directives
#
target_link_options(
    ${PROJECT_NAME}
    PUBLIC
    /SUBSYSTEM:WINDOWS

    PRIVATE
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/InferAsanLibs>>
)

target_link_libraries(
    ${PROJECT_NAME}

    PRIVATE
    ${GUI_RES_FILE}
    CFB::User::CommonLib
    imgui::imgui
    nlohmann_json::nlohmann_json
    Ws2_32.lib
    D3D12.lib
    DXGI.lib
)

#
# Compile the resource file
#
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND
    rc /nologo /fo ${GUI_RES_FILE} /r ${GUI_RC_FILE}
    COMMENT
    "Compiling '${GUI_RES_FILE}' file"
)

#
# Install directives
#
install(TARGETS ${PROJECT_NAME} DESTINATION Tools)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION Tools OPTIONAL)
