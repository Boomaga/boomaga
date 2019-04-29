 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2012-2018 Boomaga team https://github.com/Boomaga
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

project(agent)

set(MAC_APP_ID    "io.github.Boomaga")
add_definitions(-DMAC_APP_ID=\"${MAC_APP_ID}\")

set(HEADERS
    ../../common.h
)

set(SOURCES
  main.cpp
  ../../common.cpp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "agent")

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(plist.in ${CMAKE_CURRENT_BINARY_DIR}/agent.plist @ONLY)

install(TARGETS ${PROJECT_NAME}   RUNTIME 
        DESTINATION "Library/Printers/${MAC_APP_ID}")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/agent.plist         
	DESTINATION "Library/LaunchAgents"
	RENAME ${MAC_APP_ID}.agent.plist)
