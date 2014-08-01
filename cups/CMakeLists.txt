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

project(boomagaCups)

set(CUPS_BACKEND_ID "MFG:${CUPS_BACKEND_MANUFACTURER};CMD:PJL,PDF;MDL:${CUPS_BACKEND_MODEL};CLS:PRINTER;DES:${CUPS_BACKEND_DESCRIPTION};DRV:DPDF,R1,M0;")
add_definitions(-DCUPS_BACKEND_ID=\"${CUPS_BACKEND_ID}\")


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boomaga
               ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boomaga.ppd
               ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}.ppd
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boomagabackend
               ${CMAKE_CURRENT_BINARY_DIR}/boomagabackend
               @ONLY)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${INSTANCE_NAME}
  DESTINATION ${CUPS_BACKEND_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

install(FILES boomaga_pstopdf
  DESTINATION ${CUPS_FILTER_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ             GROUP_EXECUTE
    WORLD_READ             WORLD_EXECUTE
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
