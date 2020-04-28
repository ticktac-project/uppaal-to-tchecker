set(UTOT_MAJOR_VERSION 0)
set(UTOT_MINOR_VERSION 0)
set(UTOT_PATCH_VERSION 0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utot-version.hh.in
               ${CMAKE_CURRENT_BINARY_DIR}/utot-version.hh
               @ONLY)

add_executable(utot utot.cpp utot.hh utot-translate.cc utot-translate.hh
               utot-version.hh utot-expr.cc utot-contextprefix.hh utot-decl.cc utot-tchecker.hh)

target_include_directories(utot PRIVATE ${LIBUTAP_INCLUDES})
target_include_directories(utot PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(utot ${LIBUTAP_LIBRARIES})

install(TARGETS utot
        RUNTIME DESTINATION bin)
