message(STATUS "Configuring tests for 'GUI'")

enable_testing()

set(GUI_HEADERS_DIR ${CFB_ROOT_DIR}/GUI/Headers)
set(GUI_TEST_DIR ${CFB_ROOT_DIR}/GUI/Tests)

list(APPEND TEST_SET
)

foreach(TEST_FILE ${TEST_SET})
    set(FILEPATH "${GUI_TEST_DIR}/${TEST_FILE}.cpp")
    set(TARGET_NAME Tests_Gui_${TEST_FILE})
    add_executable(${TARGET_NAME} WIN32 ${FILEPATH})
    list(APPEND ALL_GUI_TESTS ${TARGET_NAME})
    add_dependencies(${TARGET_NAME} CFB::User::Gui)
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
    target_include_directories(${TARGET_NAME} PUBLIC ${GUI_HEADERS_DIR})
    target_compile_definitions(${TARGET_NAME} PRIVATE CATCH_CONFIG_NO_WINDOWS_SEH)
    target_compile_options(${TARGET_NAME} PRIVATE $<TARGET_PROPERTY:CFB::User::CommonLib,COMPILE_OPTIONS>)
    target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2 Catch2::Catch2WithMain imgui::imgui)
    target_link_options(${TARGET_NAME} PRIVATE /SUBSYSTEM:Console $<TARGET_PROPERTY:CFB::User::CommonLib,LINK_OPTIONS>)
    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    install(TARGETS ${TARGET_NAME} DESTINATION Tests/GUI)
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION Tests/GUI OPTIONAL)
endforeach()

add_custom_target(
    Tests_Gui
    DEPENDS ${ALL_GUI_TESTS}
    COMMAND ${CMAKE_CTEST_COMMAND} -R Tests_Gui
)
