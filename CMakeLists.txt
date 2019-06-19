cmake_minimum_required(VERSION 3.10)
project(uppaal_to_tcheker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

#
# UTAP Library configuration
#
###############################################################################

include(CheckUTAP)
find_or_build_utap("${CMAKE_SOURCE_DIR}/utap" "${CMAKE_BINARY_DIR}/utap")

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
              ${CMAKE_CURRENT_BINARY_DIR}/Testing)
