 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2012-2016 Boomaga team https://github.com/Boomaga
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

project(cups_backend)

set(HEADERS
    ../../common.h
)

set(SOURCES
    main.cpp
    ../../common.cpp
)



add_definitions(-DMAC_SPOOL_DIR=\"${MAC_SPOOL_DIR}\")

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "boomaga")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# MacOS ***************************************************
if(APPLE)
    set(CUPS_BACKEND_DIR "usr/libexec/cups/backend")
    set(CUPS_PPD_DIR     "Library/Printers/PPDs/Contents/Resources")
endif()


install(FILES "${CMAKE_CURRENT_BINARY_DIR}/boomaga"
  DESTINATION ${CUPS_BACKEND_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boomaga.ppd
               ${CMAKE_CURRENT_BINARY_DIR}/boomaga.ppd
               @ONLY)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/boomaga.ppd
  DESTINATION ${CUPS_PPD_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)


if (APPLE)
        configure_file(apple.postinstall.in ${CMAKE_BINARY_DIR}/scripts/postinstall @ONLY)
endif()
