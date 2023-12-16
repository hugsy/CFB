project(
    GUI
    LANGUAGES CXX
    VERSION 0.2.0
    DESCRIPTION "ImGui-based GUI for CFB"
    HOMEPAGE_URL https://github.com/hugsy/cfb
)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(GUI_SOURCE_DIR ${CFB_ROOT_DIR}/GUI/Source)
set(GUI_HEADER_DIR ${CFB_ROOT_DIR}/GUI/Headers)

set(GUI_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "GUI_VERSION_MAJOR")
set(GUI_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "GUI_VERSION_MINOR")
set(GUI_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "GUI_VERSION_PATCH")

set(
    GUI_HEADER_FILES

    ${GUI_HEADER_DIR}/App.hpp
    ${GUI_HEADER_DIR}/GuiUtils.hpp
    ${GUI_HEADER_DIR}/Helpers.hpp
    ${GUI_HEADER_DIR}/Network.hpp
)

set(
    GUI_SOURCE_FILES

    ${GUI_SOURCE_DIR}/main.cpp
    ${GUI_SOURCE_DIR}/App.cpp
    ${GUI_SOURCE_DIR}/GuiUtils.cpp
    ${GUI_SOURCE_DIR}/Helpers.cpp
)

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
add_dependencies(${PROJECT_NAME} CFB::User::CommonLib)
add_executable(CFB::User::Gui ALIAS ${PROJECT_NAME})

#
# Compilation directives
#
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC

    PRIVATE
    ${GUI_HEADER_DIR}
    $<TARGET_PROPERTY:Deps::ImGUI,INCLUDE_DIRECTORIES>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC _UNICODE UNICODE ImTextureID=ImU64)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/fsanitize=address>>

    PRIVATE
    $<IF:$<CONFIG:Debug>,/WX /Gm- /permissive-,/WX /permissive>
)

#
# Linking directives
#
target_link_options(
    ${PROJECT_NAME}
    PUBLIC
    /SUBSYSTEM:WINDOWS
    $<$<NOT:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},arm64>>:$<$<CONFIG:Debug>:/InferAsanLibs>>

    PRIVATE
)

target_link_libraries(
    ${PROJECT_NAME}

    PRIVATE
    ${GUI_RES_FILE}
    CFB::User::CommonLib
    Deps::ImGUI
    Deps::JSON
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
    "Compiling '${GUI_RES_FILE}' resource file"
)

#
# Install directives
#
install(TARGETS ${PROJECT_NAME} DESTINATION Tools)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION Tools OPTIONAL)
