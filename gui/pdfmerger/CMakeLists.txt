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


cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0028 NEW)

project(boomagamerger)

set(MERGER_HEADERS
    pdfmerger.h
    pdfprocessor.h
    pdfmergeripc.h
    ../pdfparser/pdfxref.h
    ../pdfparser/pdfvalue.h
    ../pdfparser/pdfobject.h
    ../pdfparser/pdfreader.h
    ../pdfparser/pdfwriter.h
)

set(MERGER_SOURCES
    main.cpp
    pdfmerger.cpp
    pdfprocessor.cpp
    pdfmergeripc.cpp
    ../pdfparser/pdfxref.cpp
    ../pdfparser/pdfvalue.cpp
    ../pdfparser/pdfobject.cpp
    ../pdfparser/pdfreader.cpp
    ../pdfparser/pdfwriter.cpp
)


#*******************************************
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/..")
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED
    Core
    Widgets
    PrintSupport
    LinguistTools
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CUPS_INCLUDE_DIR}
)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
set(MERGER_LIBRARIES ${MERGER_LIBRARIES} ${ZLIB_LIBRARIES})

add_executable(${PROJECT_NAME} ${MERGER_HEADERS} ${MERGER_SOURCES})
target_link_libraries(${PROJECT_NAME} ${MERGER_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::PrintSupport)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${NONGUI_DIR})



