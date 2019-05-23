set(TEST_DRIVER ${CMAKE_CURRENT_SOURCE_DIR}/test-driver.sh)
set(TESTS
    2doors.xml
    2doors2.xml
    boxes.xml
    bridge.xml
    csma_input_05.ta
    druzba1.xml
    engine-classic.xml
    fischer.xml
    fischer_input_05.ta
    fischer_symmetry.xml
    hddi_input_05.ta
    interrupt.xml
    timedsystem.xml
    train-gate.xml
    )

set (TESTS_ENVVARS
     "TCHECKER=${TCHECKER}"
     "UTOT=${UTOT}"
     "SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
     )

foreach(test ${TESTS})
    get_filename_component(testname ${test} NAME_WE)
    add_test(NAME "${testname}" COMMAND ${TEST_DRIVER} ${test})
    foreach(var ${TESTS_ENVVARS})
        set_property (TEST "${testname}" APPEND PROPERTY ENVIRONMENT ${var})
    endforeach(var ${TESTS_ENVVARS})
    set_property(TEST "${testname}" APPEND PROPERTY DEPENDS utot-build)
    foreach(ext out err cards tck.out tck.err)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                     "${CMAKE_CURRENT_BINARY_DIR}/${test}.${ext}")
    endforeach()
    unset(testname)
endforeach(test)

set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
              ${CMAKE_CURRENT_BINARY_DIR}/Testing)

add_custom_target(uppaal-examples-tests)
add_dependencies(check uppaal-examples-tests)
