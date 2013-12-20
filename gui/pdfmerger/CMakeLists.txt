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


cmake_minimum_required( VERSION 2.6 )

project(boomagamerger)

set(MERGER_HEADERS
    pdfmerger.h
)

set(MERGER_SOURCES
    main.cpp
    pdfmerger.cpp
)

set(MERGER_MOCS
    #pdfmerger.h
)



#*******************************************
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/..")
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CUPS_INCLUDE_DIR}
    ${POPPLER_INCLUDE_DIRS}
)

if(USE_QT4)
    qt4_wrap_cpp(MERGER_MOC_SOURCES ${MERGER_MOCS})
else()
    qt5_wrap_cpp(MERGER_MOC_SOURCES ${MERGER_MOCS})
endif()


add_executable(${PROJECT_NAME} ${MERGER_HEADERS} ${MERGER_SOURCES} ${MERGER_MOC_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${POPPLER_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/boomaga/)



