include(TestScripts)

add_test_suite(check "all test-suites")

if(NOT DEFINED TCHECKER)
    find_program(TCHECKER tchecker)
endif(NOT DEFINED TCHECKER)

set(UTOT "$<TARGET_FILE:utot>")

if(NOT TCHECKER)
    message(WARNING "can't find TChecker for tests.")
else()
    add_subdirectory(tchecker-examples)
    add_subdirectory(uppaal-examples)
    add_subdirectory(translation)
endif(NOT TCHECKER)



