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

project(boomagabackend)

set(HEADERS
    common.h
    inputfile.h
    systemd.h
    consolekit.h
    envinfo.h
)

set(SOURCES
    main.cpp
    common.cpp
    inputfile.cpp
    systemd.cpp
    consolekit.cpp
    envinfo.cpp
)

set(LIBRARIES
)

set (MOCS
)


if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-Wall -pedantic)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(QT_LIBRARIES, "")
if (USE_QT4)
    find_package(Qt4 REQUIRED QUIET COMPONENTS QTCORE QTDBUS)
    include(${QT_USE_FILE})
    qt4_wrap_cpp(MOC_SOURCES ${MOCS})
endif()

if (USE_QT5)
    cmake_minimum_required(VERSION 2.8.3)

    find_package(Qt5Core REQUIRED QUIET)
    include_directories(${Qt5Core_INCLUDE_DIRS})
    add_definitions(${Qt5Core_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Core_LIBRARIES})

    find_package(Qt5DBus REQUIRED QUIET)
    include_directories(${Qt5DBus_INCLUDE_DIRS})
    add_definitions(${Qt5DBus_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5DBus_LIBRARIES})

    qt5_wrap_cpp(MOC_SOURCES ${MOCS})
endif()


set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${MOC_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})


set(CUPS_BACKEND_ID "MFG:${CUPS_BACKEND_MANUFACTURER};CMD:PJL,PDF;MDL:${CUPS_BACKEND_MODEL};CLS:PRINTER;DES:${CUPS_BACKEND_DESCRIPTION};DRV:DPDF,R1,M0;")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend/boomaga
               ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend/boomaga.ppd
               ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}.ppd
               @ONLY)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}
  DESTINATION ${CUPS_BACKEND_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}.ppd
  DESTINATION ${CUPS_PPD_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/boomagabackend
  DESTINATION ${NONGUI_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ             GROUP_EXECUTE
    WORLD_READ             WORLD_EXECUTE
)
