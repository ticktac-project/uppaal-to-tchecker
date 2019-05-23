#
# Parse the "test" string to get the corresponding testname and its parameters
# that should be passed to script generators. The string follows the syntax:
# testname@arg1:arg2:...
#
function(get_test_parameters test testname testparams prefix)
    string(REPLACE "@" "_" outputprefix ${test})
    string(REPLACE ":" "_" outputprefix ${outputprefix})

    set(${prefix} ${outputprefix} PARENT_SCOPE)

    string(REPLACE "@" ";" test ${test})
    list(GET test 0 name)
    set(${testname} ${name} PARENT_SCOPE)

    list(LENGTH test testlen)
    if (${testlen} GREATER 1)
        list(GET test 1 test)
        string(REPLACE ":" " " test "${test}")
        set(${testparams} "${test}" PARENT_SCOPE)
    endif()

endfunction()


set(TEST_DRIVER ${CMAKE_CURRENT_SOURCE_DIR}/test-driver.sh)
set(TESTS
    ad94@
    critical-region-async@3
    critical-region@3
    csmacd@2
    csmacd@6:100:50
    csmacd@7:50:100
    dining-philosophers@5
    dining-philosophers@7:12:13:1
    fddi@5
    fddi@13
    fire-alarm@5
    fire-alarm@5:13:500
    fischer-async@7
    fischer@7
    leader-election-async@5:4
    leader-election@5:4
    parallel-b@10
    parallel-c@10
    parallel@10
    train_gate@5
#    corsso@3 to fixed with parenthesis
    )

set (TESTS_ENVVARS
     "TCHECKER=${TCHECKER}"
     "UTOT=${UTOT}"
     "SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
     )

foreach(test ${TESTS})
    get_test_parameters(${test} testname testparams prefix)
    add_test(NAME "${test}" COMMAND ${TEST_DRIVER} ${testname} ${testparams})
    foreach(var ${TESTS_ENVVARS})
        set_property (TEST "${test}" APPEND PROPERTY ENVIRONMENT ${var})
    endforeach(var ${TESTS_ENVVARS})
    set_property(TEST "${test}" APPEND PROPERTY DEPENDS utot-build)
    foreach(ext out res tck cards)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                     "${CMAKE_CURRENT_BINARY_DIR}/${prefix}-utot.${ext}")
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                     "${CMAKE_CURRENT_BINARY_DIR}/${prefix}-direct.${ext}")
    endforeach()
    unset(testname)
    unset(testparams)
    unset(prefix)
endforeach(test)
set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
              ${CMAKE_CURRENT_BINARY_DIR}/Testing)

add_custom_target(tchecker-examples-tests ALL)
add_dependencies(check tchecker-examples-tests)
