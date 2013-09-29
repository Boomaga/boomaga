cmake_minimum_required( VERSION 2.6 )

project(boomaga)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 2)
set(PATCH_VERSION 0)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

add_definitions(-DMAJOR_VERSION=\"${MAJOR_VERSION}\")
add_definitions(-DMINOR_VERSION=\"${MINOR_VERSION}\")
add_definitions(-DPATCH_VERSION=\"${PATCH_VERSION}\")
add_definitions(-DFULL_VERSION=\"${FULL_VERSION}\")

find_program(CUPS_CONFIG_EXECUTABLE NAMES cups-config)
if (CUPS_CONFIG_EXECUTABLE)
    exec_program(${CUPS_CONFIG_EXECUTABLE} ARGS --serverbin OUTPUT_VARIABLE CUPS_SERVERBIN)
    set(DEF_CUPS_BACKEND_DIR "${CUPS_SERVERBIN}/backend")
else(CUPS_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "Can't find cups-config. Are CUPS development packages installed?")
endif(CUPS_CONFIG_EXECUTABLE)

# Linux ***************************************
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

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

set(GUI_PROGRAM ${GUI_DIR}/boomaga)
add_definitions(-DGUI_PROGRAM=\"${GUI_PROGRAM}\")

set(GUI_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/boomaga)
add_definitions(-DGUI_SHARE_DIR=\"${GUI_SHARE_DIR}\")



set(CUPS_BACKEND_URI "boomaga:/")
add_definitions(-DCUPS_BACKEND_URI=\"${CUPS_BACKEND_URI}\")


set(CUPS_BACKEND_INFO "Boomaga")
add_definitions(-DCUPS_BACKEND_INFO=\"${CUPS_BACKEND_NAME}\")


set(CUPS_BACKEND_MODEL "Virtual boomaga printer")
add_definitions(-DCUPS_BACKEND_MODEL=\"${CUPS_BACKEND_MODEL}\")


set(CUPS_BACKEND_MANUFACTURER "Generic")
add_definitions(-DCUPS_BACKEND_MANUFACTURER=\"${CUPS_BACKEND_MANUFACTURER}\")



###########################################################

macro(setByDefault VAR_NAME VAR_VALUE)
  if (NOT DEFINED ${VAR_NAME})
    set (${VAR_NAME} ${VAR_VALUE})
    add_definitions(-D${VAR_NAME}=\"${VAR_VALUE}\")
  endif()
endmacro()

setByDefault(CUPS_BACKEND_DIR   ${DEF_CUPS_BACKEND_DIR})
setByDefault(CUPS_PPD_DIR       ${DEF_CUPS_PPD_DIR})
setByDefault(DBUS_MACINE_ID_DIR ${DEF_DBUS_MACINE_ID_DIR})
setByDefault(DBUS_SERVICE_DIR   ${DEF_DBUS_SERVICE_DIR})


add_subdirectory(cups)
add_subdirectory(gui)
install(FILES boomaga.1.gz  DESTINATION share/man/man1)

message(STATUS "*****************************************************")
message(STATUS "* boomaga ver. ${FULL_VERSION}")
message(STATUS "*")
message(STATUS "* You can change the following directories using cmake options like:")
message(STATUS "*   cmake -DCUPS_PPD_DIR=your_path")
message(STATUS "*")
message(STATUS "* CUPS_PPD_DIR       = ${CUPS_PPD_DIR}")
message(STATUS "* DBUS_MACINE_ID_DIR = ${DBUS_MACINE_ID_DIR}")
message(STATUS "* DBUS_SERVICE_DIR   = ${DBUS_SERVICE_DIR}")
message(STATUS "*****************************************************")

########## Add make dist target ################
SET(CPACK_PACKAGE_VERSION "${FULL_VERSION}")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "boomaga-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES
    \\\\.git
    build
    CMakeLists.txt.user
    \\\\~
)
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)