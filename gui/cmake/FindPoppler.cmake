 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
 # Authors:
 #   Alexander Sokoloff <sokoloff.a@gmail.com>
 #
 # This program or library is free software; you can redistribute it
 # and/or modify it under the terms of the GNU Lesser General Public
 # License as published by the Free Software Foundation; either
 # version 2.1 of the License, or (at your option) any later version.
 #
 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.
 #
 # You should have received a copy of the GNU Lesser General
 # Public License along with this library; if not, write to the
 # Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 # Boston, MA 02110-1301 USA
 #
 # END_COMMON_COPYRIGHT_HEADER

#
# Find snappy compression library and includes. This module defines:
#   POPPLER_INCLUDE_DIR - The directories containing snappy's headers.
#   POPPLER_LIBRARY    - A list of snappy's libraries.
#   POPPLER_FOUND        - Whether snappy was found.

include(FindPkgConfig)

pkg_search_module(POPPLER REQUIRED QUIET poppler)
link_directories(${POPPLER_LIBRARY_DIRS})

if(POPPLER_FOUND)
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\1" POPPLER_MAJOR_VERSION ${POPPLER_VERSION})
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\2" POPPLER_MINOR_VERSION ${POPPLER_VERSION})
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\3" POPPLER_PATCH_VERSION ${POPPLER_VERSION})

    math(EXPR POPPLER_VER_INT "(10000 * ${POPPLER_MAJOR_VERSION}) + (100 * ${POPPLER_MINOR_VERSION}) + (${POPPLER_PATCH_VERSION})")
    add_definitions(-DPOPPLER_VERSION=${POPPLER_VER_INT})

    pkg_search_module(POPPLERCPP REQUIRED QUIET poppler-cpp)
    link_directories(${POPPLERCPP_LIBRARY_DIRS})
endif()
