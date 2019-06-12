add_test_suite(check-uppaal-examples "Uppaal Examples")

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
    add_test(NAME "${test}" COMMAND ${TEST_DRIVER} ${test})
    foreach(var ${TESTS_ENVVARS})
        set_property (TEST "${test}" APPEND PROPERTY ENVIRONMENT ${var})
    endforeach(var ${TESTS_ENVVARS})
    set_property(TEST "${test}" APPEND PROPERTY DEPENDS utot-build)
    foreach(ext out err cards tck.out tck.err)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                     "${CMAKE_CURRENT_BINARY_DIR}/${test}.${ext}")
    endforeach()
endforeach(test)
