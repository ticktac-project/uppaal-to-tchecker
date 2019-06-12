set(TEST_DRIVER ${CMAKE_CURRENT_SOURCE_DIR}/test-driver.sh)
file (GLOB TESTS
      LIST_DIRECTORIES false
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
      "*.xta")

add_test_suite(check-translation "Basic translations" ${TEST_DRIVER} ${TESTS})

set (TESTS_ENVVARS
     "UTOT=${UTOT}"
     "TCHECKER=${TCHECKER}"
     "SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
     )

foreach(test ${TESTS})
    add_test(NAME "${test}" COMMAND ${TEST_DRIVER} ${test})
    foreach(var ${TESTS_ENVVARS})
        set_property (TEST "${test}" APPEND PROPERTY ENVIRONMENT ${var})
    endforeach(var ${TESTS_ENVVARS})
    set_property(TEST "${test}" APPEND PROPERTY DEPENDS utot-build)
    foreach(ext tck.out tck.err cards err out)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                     "${CMAKE_CURRENT_BINARY_DIR}/${test}.${ext}")
    endforeach()
endforeach(test)
