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
 
 
 project(boomaga_test)
cmake_minimum_required( VERSION 2.6 )


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

set(TEST_MOCS
    testboomaga.h
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
ADD_FROM_PROJECT(TEST_MOCS    ${MOCS})


add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\")

set(TEST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output/")
file(MAKE_DIRECTORY ${TEST_OUT_DIR})
add_definitions(-DTEST_OUT_DIR=\"${TEST_OUT_DIR}\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(USE_QT4)
    set(QT_USE_QTTEST true)
    find_package(Qt4 REQUIRED QUIET)
    include(${QT_USE_FILE})
    qt4_wrap_cpp(TEST_MOC_SOURCES ${TEST_MOCS})
else()
    cmake_minimum_required(VERSION 2.8.3)

    find_package(Qt5Test REQUIRED)
    include_directories(${Qt5Test_INCLUDE_DIRS})
    add_definitions(${Qt5Test_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Test_LIBRARIES})

    qt5_wrap_cpp(TEST_MOC_SOURCES ${TEST_MOCS})
endif()


add_executable(${PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES} ${TEST_MOC_SOURCES} )
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${LIBRARIES})
