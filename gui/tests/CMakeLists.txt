 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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
 
 
cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0028 NEW)

project(boomaga_test)


set(TEST_HEADERS
    testboomaga.h
    tools.h
    ../pdfparser/pdfreader.h
    ../pdfparser/pdfvalue.h
    ../pdfparser/pdfdata.h
    ../pdfparser/pdftypes.h
    ../pdfparser/pdfobject.h

)

set(TEST_SOURCES
    testboomaga.cpp
    testpdfparser.cpp
    ../pdfparser/pdfreader.cpp
    ../pdfparser/pdfvalue.cpp
    ../pdfparser/pdfdata.cpp
    ../pdfparser/pdfobject.cpp
)


macro(ADD_FROM_PROJECT OUT_FILES)
    set (IN_FILES ${ARGN})
    foreach(FILE ${IN_FILES})
        if(NOT FILE STREQUAL "main.cpp")
            message(">>>${FILE}<<<")
            set(${OUT_FILES} ${${OUT_FILES}} "../${FILE}")
        endif()
    endforeach()
endmacro()


ADD_FROM_PROJECT(TEST_HEADERS ${HEADERS})
ADD_FROM_PROJECT(TEST_SOURCES ${SOURCES})


add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\")

set(TEST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output/")
file(MAKE_DIRECTORY ${TEST_OUT_DIR})
add_definitions(-DTEST_OUT_DIR=\"${TEST_OUT_DIR}\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(Qt5 REQUIRED
    Core
    Test
    Widgets
    PrintSupport
    DBus
    LinguistTools
)


add_executable(${PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} Qt5::Core Qt5::Test Qt5::Widgets Qt5::PrintSupport Qt5::DBus)
