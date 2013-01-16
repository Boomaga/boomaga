cmake_minimum_required( VERSION 2.6 )

project(booklet)

set(GUI_DIR ${CMAKE_INSTALL_PREFIX}/lib/booklet)
add_definitions(-DGUI_DIR=\"${GUI_DIR}\")

set(GUI_PROGRAM ${GUI_DIR}/booklet)
add_definitions(-DGUI_PROGRAM=\"${GUI_PROGRAM}\")

set(MY_CUPS_URI "booklet:/")
add_definitions(-DMY_CUPS_URI=\"${MY_CUPS_URI}\")

add_subdirectory(cups)
add_subdirectory(gui)

