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


cmake_minimum_required(VERSION 3.25.0)
cmake_policy(SET CMP0028 NEW)

project(boomaga)

set(MAJOR_VERSION 3)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)
#set(BETA_VERSION beta5)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
if (BETA_VERSION)
    set(FULL_VERSION ${FULL_VERSION}.${BETA_VERSION})
endif()


add_definitions(-DMAJOR_VERSION=${MAJOR_VERSION})
add_definitions(-DMINOR_VERSION=${MINOR_VERSION})
add_definitions(-DPATCH_VERSION=${PATCH_VERSION})
add_definitions(-DFULL_VERSION=\"${FULL_VERSION}\")

set (CMAKE_CXX_STANDARD 11)

include("cmake/tools.cmake")
include("cmake/status_message.cmake")
include("cmake/cxx_standard.cmake")

status_message("boomaga ver. ${FULL_VERSION}")
status_message(" ")


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_program(CUPS_CONFIG_EXECUTABLE NAMES cups-config)
if(CUPS_CONFIG_EXECUTABLE)
    exec_program(${CUPS_CONFIG_EXECUTABLE} ARGS --serverbin OUTPUT_VARIABLE CUPS_SERVERBIN)
    set(DEF_CUPS_BACKEND_DIR "${CUPS_SERVERBIN}/backend")
else(CUPS_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "Can't find cups-config. Are CUPS development packages installed?")
endif(CUPS_CONFIG_EXECUTABLE)

# Linux ***************************************
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

  set(DEF_CUPS_PPD_DIR /usr/share/ppd/boomaga)
  set(DEF_DBUS_SERVICE_DIR ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
  set(USE_DBUS TRUE)
  set(GUI_DIR ${CMAKE_INSTALL_PREFIX}/bin)
  set(NONGUI_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/boomaga)

# FreeBSD *************************************
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")

  set(DEF_CUPS_PPD_DIR /usr/local/share/cups/model)
  set(DEF_DBUS_SERVICE_DIR ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
  set(USE_DBUS TRUE)
  set(GUI_DIR ${CMAKE_INSTALL_PREFIX}/bin)
  set(NONGUI_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/boomaga)

# MacOS ***************************************
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

  set(MAC_APP_ID    "io.github.Boomaga")
  set(MAC_SPOOL_DIR "/var/spool/${MAC_APP_ID}")

  set(DEF_CUPS_PPD_DIR ${CMAKE_INSTALL_PREFIX}/Library/Printers/PPDs/Contents/Resources)
  set(GUI_DIR    ${CMAKE_INSTALL_PREFIX}/Applications/Boomaga.app/Contents/MacOS)
  set(NONGUI_DIR ${CMAKE_INSTALL_PREFIX}/Applications/Boomaga.app/Contents/MacOS)
  set(USE_DBUS FALSE)

endif()

set(AUTOREMOVE_EXT "autoremove")
add_definitions(-DAUTOREMOVE_EXT=\"${AUTOREMOVE_EXT}\")

add_definitions(-DGUI_DIR=\"${GUI_DIR}\")
add_definitions(-DNONGUI_DIR=\"${NONGUI_DIR}\")

if (USE_DBUS)
  add_definitions(-DUSE_DBUS=1)
endif()


###########################################################

setFirstUpper(CUPS_BACKEND_NAME "boomaga")

setByDefault(CUPS_BACKEND_URI   "boomaga:/")
setByDefault(CUPS_BACKEND_INFO  "${CUPS_BACKEND_NAME}")
setByDefault(CUPS_BACKEND_MODEL "${CUPS_BACKEND_NAME} printer")
setByDefault(CUPS_BACKEND_DESCRIPTION  "Virtual boomaga printer")
setByDefault(CUPS_BACKEND_MANUFACTURER "Generic")

setByDefault(GUI_PROGRAM    ${GUI_DIR}/boomaga)
setByDefault(GUI_SHARE_DIR  ${CMAKE_INSTALL_PREFIX}/share/boomaga)

setByDefault(CUPS_BACKEND_DIR   "${DEF_CUPS_BACKEND_DIR}")
setByDefault(CUPS_PPD_DIR       "${DEF_CUPS_PPD_DIR}")
setByDefault(DBUS_SERVICE_DIR   "${DEF_DBUS_SERVICE_DIR}")


status_message(" ")
status_message("You can change the following directories using cmake options like:")
status_message("-DCUPS_PPD_DIR=your_path")
status_message(" ")
status_message("  CUPS_BACKEND_DIR     =  ${CUPS_BACKEND_DIR}")
status_message("  CUPS_PPD_DIR         =  ${CUPS_PPD_DIR}")
status_message("  DBUS_SERVICE_DIR     =  ${DBUS_SERVICE_DIR}")
status_message(" ")
status_message(" You can change the following options using cmake options:")
status_message("  CUPS_BACKEND_URI     =  ${CUPS_BACKEND_URI}")
status_message("  CUPS_BACKEND_INFO    =  ${CUPS_BACKEND_INFO}")
status_message("  CUPS_BACKEND_MODEL   =  ${CUPS_BACKEND_MODEL}")
status_message("  CUPS_BACKEND_DESCRIPTION  = ${CUPS_BACKEND_DESCRIPTION}")
status_message("  CUPS_BACKEND_MANUFACTURER = ${CUPS_BACKEND_MANUFACTURER}")
status_message(" ")

add_subdirectory(src/backend)
add_subdirectory(src/boomaga)
add_subdirectory(selinux)

show_status()
