add_executable(utot main.cpp)

target_include_directories(utot PRIVATE ${LIBUTAP_INCLUDES})
target_link_libraries(utot ${LIBUTAP_LIBRARIES})
