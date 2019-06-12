macro(add_utot_build_test)
    if (NOT TEST utot-build)
        add_test(NAME utot-build
                 COMMAND ${CMAKE_COMMAND} --build . --config "$<CONFIG>" --target utot
                 WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif ()
endmacro()

macro(add_test_suite testsuite comment)
    add_utot_build_test()
    add_custom_target(${testsuite}
                      ${CMAKE_CTEST_COMMAND}
                      COMMENT "Starting test-suite ${testsuite}. ${comment}")

    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                 ${CMAKE_CURRENT_BINARY_DIR}/Testing)
    if (NOT "${testsuite}" STREQUAL "check")
            add_dependencies(check ${testsuite})
    endif ()
endmacro()
