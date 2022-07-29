project(Broker LANGUAGES CXX VERSION 0.2.0)

message(STATUS "Configuring '${PROJECT_NAME}'")

set(BROKER_SOURCE_DIR ${CFB_ROOT_DIR}/Broker)
set(BROKER_HEADER_DIR ${BROKER_SOURCE_DIR})

set(BROKER_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "BROKER_VERSION_MAJOR")
set(BROKER_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "BROKER_VERSION_MINOR")
set(BROKER_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "BROKER_VERSION_PATCH")

#
# Source files
#
set(
    HEADER_FILES
)

set(
    SOURCE_FILES

    ${BROKER_SOURCE_DIR}/Main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
add_dependencies(${PROJECT_NAME} IrpMonitor)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${BROKER_HEADER_DIR}
    CommonLibUser
    ${CFB_DEPENDENCIES}/wil/include
    ${CFB_DEPENDENCIES}/argparse
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME})

if(DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG=1 DEBUG=1)
endif(DEBUG)

#
# Compilation directives
#
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

#
# Linking directives
#
target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:Console)
target_link_libraries(${PROJECT_NAME} CommonLibUser)

#
# Custom command directives
#
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    echo "todo embed driver"
)
