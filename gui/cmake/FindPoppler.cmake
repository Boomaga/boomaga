# This is code is released under the
# Apache License Version 2.0 http://www.apache.org/licenses/.
#
# Copyright (c) 2013 Alex Sokolov
#
# Find snappy compression library and includes. This module defines:
#   POPPLER_INCLUDE_DIR - The directories containing snappy's headers.
#   POPPLER_LIBRARY    - A list of snappy's libraries.
#   POPPLER_FOUND        - Whether snappy was found.

include(FindPkgConfig)
pkg_search_module(POPPLER REQUIRED QUIET poppler)


string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\1" POPPLER_MAJOR_VERSION ${POPPLER_VERSION})
string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\2" POPPLER_MINOR_VERSION ${POPPLER_VERSION})
string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\3" POPPLER_PATCH_VERSION ${POPPLER_VERSION})

math(EXPR POPPLER_VER_INT "(10000 * ${POPPLER_MAJOR_VERSION}) + (100 * ${POPPLER_MINOR_VERSION}) + (${POPPLER_PATCH_VERSION})")
add_definitions(-DPOPPLER_VERSION=${POPPLER_VER_INT})


pkg_search_module(POPPLERQT REQUIRED QUIET poppler-qt4)

