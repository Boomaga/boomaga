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

project(boomaga)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 2)
set(PATCH_VERSION 0)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

add_definitions(-DMAJOR_VERSION=\"${MAJOR_VERSION}\")
add_definitions(-DMINOR_VERSION=\"${MINOR_VERSION}\")
add_definitions(-DPATCH_VERSION=\"${PATCH_VERSION}\")
add_definitions(-DFULL_VERSION=\"${FULL_VERSION}\")

set (CMAKE_CXX_STANDARD 11)

include("cmake/tools.cmake")
status_message("boomaga ver. ${FULL_VERSION}")
status_message(" ")


find_program(CUPS_CONFIG_EXECUTABLE NAMES cups-config)
if(CUPS_CONFIG_EXECUTABLE)
    exec_program(${CUPS_CONFIG_EXECUTABLE} ARGS --serverbin OUTPUT_VARIABLE CUPS_SERVERBIN)
    set(DEF_CUPS_BACKEND_DIR "${CUPS_SERVERBIN}/backend")
    set(DEF_CUPS_FILTER_DIR  "${CUPS_SERVERBIN}/filter")
else(CUPS_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "Can't find cups-config. Are CUPS development packages installed?")
endif(CUPS_CONFIG_EXECUTABLE)

# Linux ***************************************
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

  set(DEF_CUPS_PPD_DIR /usr/share/ppd/boomaga)
  set(DEF_DBUS_MACINE_ID_DIR /var/lib/dbus)
  set(DEF_DBUS_SERVICE_DIR ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

# FreeBSD *************************************
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")

  set(DEF_CUPS_PPD_DIR /usr/local/share/cups/model)
  set(DEF_DBUS_MACINE_ID_DIR /var/db/dbus)
  set(DEF_DBUS_SERVICE_DIR ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

endif()

set(GUI_DIR ${CMAKE_INSTALL_PREFIX}/bin)
add_definitions(-DGUI_DIR=\"${GUI_DIR}\")

set(NONGUI_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/boomaga)
add_definitions(-DNONGUI_DIR=\"${NONGUI_DIR}\")

###########################################################

setByDefault(INSTANCE_NAME      "boomaga")
setFirstUpper(CUPS_BACKEND_NAME "${INSTANCE_NAME}")

setByDefault(CUPS_BACKEND_URI   "${INSTANCE_NAME}:/")
setByDefault(CUPS_BACKEND_INFO  "${CUPS_BACKEND_NAME}")
setByDefault(CUPS_BACKEND_MODEL "${CUPS_BACKEND_NAME} printer")
setByDefault(CUPS_BACKEND_DESCRIPTION  "Virtual ${INSTANCE_NAME} printer")
setByDefault(CUPS_BACKEND_MANUFACTURER "Generic")

setByDefault(GUI_PROGRAM    ${GUI_DIR}/${INSTANCE_NAME})
setByDefault(GUI_SHARE_DIR  ${CMAKE_INSTALL_PREFIX}/share/${INSTANCE_NAME})

setByDefault(CUPS_BACKEND_DIR   "${DEF_CUPS_BACKEND_DIR}")
setByDefault(CUPS_FILTER_DIR    "${DEF_CUPS_FILTER_DIR}")
setByDefault(CUPS_PPD_DIR       "${DEF_CUPS_PPD_DIR}")
setByDefault(DBUS_MACINE_ID_DIR "${DEF_DBUS_MACINE_ID_DIR}")
setByDefault(DBUS_SERVICE_DIR   "${DEF_DBUS_SERVICE_DIR}")


status_message(" ")
status_message("You can change the following directories using cmake options like:")
status_message("-DCUPS_PPD_DIR=your_path")
status_message(" ")
status_message("  CUPS_BACKEND_DIR     =  ${CUPS_BACKEND_DIR}")
status_message("  CUPS_FILTER_DIR      =  ${CUPS_FILTER_DIR}")
status_message("  CUPS_PPD_DIR         =  ${CUPS_PPD_DIR}")
status_message("  DBUS_MACINE_ID_DIR   =  ${DBUS_MACINE_ID_DIR}")
status_message("  DBUS_SERVICE_DIR     =  ${DBUS_SERVICE_DIR}")
status_message(" ")
status_message(" You can change the following options using cmake options:")
status_message("  INSTANCE_NAME        =  ${INSTANCE_NAME}")
status_message("  CUPS_BACKEND_URI     =  ${CUPS_BACKEND_URI}")
status_message("  CUPS_BACKEND_INFO    =  ${CUPS_BACKEND_INFO}")
status_message("  CUPS_BACKEND_MODEL   =  ${CUPS_BACKEND_MODEL}")
status_message("  CUPS_BACKEND_DESCRIPTION  = ${CUPS_BACKEND_DESCRIPTION}")
status_message("  CUPS_BACKEND_MANUFACTURER = ${CUPS_BACKEND_MANUFACTURER}")
status_message(" ")

add_subdirectory(src/backend)
add_subdirectory(src/boomaga)

show_status()
