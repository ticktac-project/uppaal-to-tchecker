add_custom_target(check ${CMAKE_COMMAND} --build . --target test)

if(NOT DEFINED TCHECKER)
    find_program(TCHECKER tchecker)
endif(NOT DEFINED TCHECKER)

add_test(NAME utot-build
         COMMAND ${CMAKE_COMMAND} --build . --config "$<CONFIG>" --target utot
         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

set(UTOT "$<TARGET_FILE:utot>")

if(NOT TCHECKER)
    message(WARNING "can't find TChecker for tests.")
    return()
else()
    add_subdirectory(tchecker-examples)
    add_subdirectory(uppaal-examples)
endif(NOT TCHECKER)
