cmake_minimum_required( VERSION 2.6 )

project(boomaga)

set(GUI_DIR ${CMAKE_INSTALL_PREFIX}/lib/boomaga)
add_definitions(-DGUI_DIR=\"${GUI_DIR}\")

set(GUI_PROGRAM ${GUI_DIR}/boomaga)
add_definitions(-DGUI_PROGRAM=\"${GUI_PROGRAM}\")

set(CUPS_BACKEND_URI "boomaga:/")
add_definitions(-DCUPS_BACKEND_URI=\"${CUPS_BACKEND_URI}\")

set(CUPS_BACKEND_MANUFACTURER "Generic")
add_definitions(-DCUPS_BACKEND_MANUFACTURER=\"${CUPS_BACKEND_MANUFACTURER}\")

set(CUPS_BACKEND_MODEL "Boomaga Printer")
add_definitions(-DCUPS_BACKEND_MODEL=\"${CUPS_BACKEND_MODEL}\")


add_subdirectory(cups)
add_subdirectory(gui)

