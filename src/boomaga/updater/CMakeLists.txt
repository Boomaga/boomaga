 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2019 Boomaga team https://github.com/Boomaga
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


project(updater_mac)

set(HEADERS 
    updater.h
)

set(SOURCES
    updater.mm
)

add_definitions(-DMAC_UPDATER=1)
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(Sparkle REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SPARKLE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${SPARKLE_LIBRARY})


FIND_LIBRARY(COCOA_LIBRARY Cocoa)
target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY})


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED Core)
target_link_libraries(${PROJECT_NAME} Qt5::Core)




