message(STATUS "Configuring tests for 'CommonLib'")

enable_testing()

set(COMMON_HEADERS_DIR ${CFB_ROOT_DIR}/Common/Headers)
set(COMMON_TEST_DIR ${CFB_ROOT_DIR}/Common/Tests)

set(TEST_SET
    Comms
    Utils
)

foreach(TEST_FILE ${TEST_SET})
    set(FILEPATH "${COMMON_TEST_DIR}/${TEST_FILE}.cpp")
    set(TARGET_NAME Tests_Common_${TEST_FILE})
    add_executable(${TARGET_NAME} WIN32 ${FILEPATH})
    list(APPEND ALL_COMMON_TESTS ${TARGET_NAME})
    add_dependencies(${TARGET_NAME} CFB::User::CommonLib)
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
    target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_HEADERS_DIR})
    target_compile_definitions(${TARGET_NAME} PRIVATE CATCH_CONFIG_NO_WINDOWS_SEH)
    target_compile_options(${TARGET_NAME} PRIVATE $<TARGET_PROPERTY:CFB::User::CommonLib,COMPILE_OPTIONS>)
    target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2 Catch2::Catch2WithMain CFB::User::CommonLib)
    target_link_options(${TARGET_NAME} PRIVATE /SUBSYSTEM:Console $<TARGET_PROPERTY:CFB::User::CommonLib,LINK_OPTIONS>)
    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    install(TARGETS ${TARGET_NAME} DESTINATION Tests/Common)
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION Tests/Common OPTIONAL)
endforeach()

add_custom_target(
    Tests_Common
    DEPENDS ${ALL_COMMON_TESTS}
    COMMAND ${CMAKE_CTEST_COMMAND} -R Tests_Common
)
