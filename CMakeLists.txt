# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

# pull in idf specific items, config vars etc.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ctag-straempler)

# generate include definitions for hw + fw version
execute_process(
        COMMAND bash -c "git describe --tags --abbrev=4"
        OUTPUT_VARIABLE STRAMPLER_FW)
string(STRIP ${STRAMPLER_FW} STRAMPLER_FW)
configure_file(${CMAKE_SOURCE_DIR}/strampler_version.h.in ${CMAKE_BINARY_DIR}/gen_include/strampler_version.h @ONLY)
idf_build_set_property(INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/gen_include APPEND)
