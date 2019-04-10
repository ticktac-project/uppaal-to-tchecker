set(UTOT_MAJOR_VERSION 0)
set(UTOT_MINOR_VERSION 0)
set(UTOT_PATCH_VERSION 0)

add_executable(utot utot.cpp utot.hh)

target_include_directories(utot PRIVATE ${LIBUTAP_INCLUDES})
target_link_libraries(utot ${LIBUTAP_LIBRARIES})
target_compile_definitions(utot PRIVATE UTOT_MAJOR_VERSION=${UTOT_MAJOR_VERSION})
target_compile_definitions(utot PRIVATE UTOT_MINOR_VERSION=${UTOT_MINOR_VERSION})
target_compile_definitions(utot PRIVATE UTOT_PATCH_VERSION=${UTOT_PATCH_VERSION})
