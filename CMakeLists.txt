cmake_minimum_required(VERSION 3.12)
project(uppaal_to_tcheker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

#
# UTAP Library configuration
#
###############################################################################

include(CheckUTAP)
find_or_build_utap("${CMAKE_SOURCE_DIR}/utap" "${CMAKE_BINARY_DIR}/utap")

add_subdirectory(src)
add_subdirectory(tests)