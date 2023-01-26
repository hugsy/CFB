# FindImGui.cmake
#
# Finds ImGui library
#
# This will define the following variables
# IMGUI_FOUND
# IMGUI_INCLUDE_DIR
# IMGUI_SOURCES
# IMGUI_VERSION

list(APPEND IMGUI_SEARCH_PATH ${IMGUI_DIR})

find_path(IMGUI_INCLUDE_DIR
    NAMES imgui.h
    PATHS
    ${IMGUI_SEARCH_PATH}
    /include
    /usr/include
    /usr/local/include
)

if(NOT IMGUI_INCLUDE_DIR)
    message(FATAL_ERROR "ImGui not found")
endif()

# Extract version from header
file(
    STRINGS
    ${IMGUI_INCLUDE_DIR}/imgui.h
    IMGUI_VERSION
    REGEX "#define IMGUI_VERSION "
)

if(NOT IMGUI_VERSION)
    message(FATAL_ERROR "Failed to determine ImGui version")
endif()

string(REGEX REPLACE ".*\"(.*)\".*" "\\1" IMGUI_VERSION "${IMGUI_VERSION}")

# TODO: compare version

# build the static library for the given backend
set(IMGUI_HEADER_FILES
    ${IMGUI_INCLUDE_DIR}/imconfig.h
    ${IMGUI_INCLUDE_DIR}/imgui.h
    ${IMGUI_INCLUDE_DIR}/imgui_internal.h
)

set(IMGUI_SOURCE_FILES
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
)

if(${IMGUI_BACKEND} STREQUAL "win32-dx12")
    set(IMGUI_HEADER_FILES ${IMGUI_HEADER_FILES} ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_dx12.h ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_win32.h)
    set(IMGUI_SOURCE_FILES ${IMGUI_SOURCE_FILES} ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_dx12.cpp ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_win32.cpp)
    set(IMGUI_LINK_LIBS d3d12 d3dcompiler dxgi)

else()
    message(FATAL_ERROR "Unsupported GUI backend")
endif()

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
target_link_libraries(imgui PUBLIC ${IMGUI_LINK_LIBS})
add_library(imgui::imgui ALIAS imgui)

message(STATUS "Found ImGui ${IMGUI_VERSION} in \"${IMGUI_INCLUDE_DIR}\"")
