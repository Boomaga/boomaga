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


cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0028 NEW)

project(boomagaGui)

set(HEADERS
    kernel/sheet.h
    kernel/inputfile.h
    kernel/job.h
    kernel/tmppdffile.h
    kernel/layout.h
    render.h
    kernel/project.h
    kernel/projectpage.h
    kernel/printer.h
    kernel/cupsprinteroptions.h
    boomagatypes.h
    mainwindow.h
    actions.h
    settings.h
    dbus.h
    icon.h
    printersettings/printersettings.h
    aboutdialog/aboutdialog.h
    translations/translatorsinfo/translatorsinfo.h
    export/exporttopdf.h
    kernel/boomagapoppler.h
    kernel/projectfile.h
    pdfmerger/pdfmergeripc.h
    configdialog/configdialog.h
    printdialog/printdialog.h
    widgets/pagelistview.h
    widgets/joblistview.h
    widgets/layoutradiobutton.h
    widgets/printerscombobox.h
    widgets/previewwidget.h
    widgets/subbookletview.h
)

set(SOURCES
    main.cpp
    kernel/sheet.cpp
    kernel/inputfile.cpp
    kernel/job.cpp
    kernel/tmppdffile.cpp
    kernel/layout.cpp
    render.cpp
    kernel/project.cpp
    kernel/projectpage.cpp
    kernel/cupsprinteroptions.cpp
    kernel/printer.cpp
    boomagatypes.cpp
    mainwindow.cpp
    actions.cpp
    settings.cpp
    dbus.cpp
    icon.cpp
    printersettings/printersettings.cpp
    aboutdialog/aboutdialog.cpp
    translations/translatorsinfo/translatorsinfo.cpp
    export/exporttopdf.cpp
    kernel/boomagapoppler.cpp
    kernel/projectfile.cpp
    pdfmerger/pdfmergeripc.cpp
    configdialog/configdialog.cpp
    printdialog/printdialog.cpp
    widgets/pagelistview.cpp
    widgets/joblistview.cpp
    widgets/layoutradiobutton.cpp
    widgets/printerscombobox.cpp
    widgets/previewwidget.cpp
    widgets/subbookletview.cpp
)


set(LIBRARIES
)

set(QT_USE_QTDBUS 1)

set(UI_FILES
    mainwindow.ui
    printersettings/printersettings.ui
    aboutdialog/aboutdialog.ui
    export/exporttopdf.ui
    configdialog/configdialog.ui
    printdialog/printdialog.ui
)

# QRC files .................
set(RESOURCES
    resource.qrc
)

# add translations ...
file (GLOB TS_FILES
    translations/boomaga*.ts
)

set(TRANSLATIONS_DIR
    "${GUI_SHARE_DIR}/translations"
)
add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")


############################################
if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-Wall -pedantic)
endif()

#*******************************************

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED
    Core
    Widgets
    PrintSupport
    DBus
    LinguistTools
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)


SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Cups REQUIRED QUIET)
include_directories(${CUPS_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${CUPS_LIBRARIES})

find_package(Poppler REQUIRED QUIET)
include_directories(${POPPLER_INCLUDE_DIRS} ${POPPLERCPP_INCLUDE_DIRS})
set(LIBRARIES ${LIBRARIES} ${POPPLER_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${POPPLERCPP_LIBRARIES})


include(translations/translatorsinfo/CMakeLists.txt)
get_translatorsinfo_qrc(translatorsinfo_qrc)
qt5_add_resources(TRANSLATORS_INFO_QRC ${translatorsinfo_qrc})

qt5_add_resources(  QRC_SOURCES ${RESOURCES})
qt5_add_translation(QM_FILES    ${TS_FILES})


add_executable(${INSTANCE_NAME} ${HEADERS} ${SOURCES} ${QM_FILES} ${QRC_SOURCES} ${TRANSLATORS_INFO_QRC})
target_link_libraries(${INSTANCE_NAME} ${LIBRARIES} Qt5::Widgets Qt5::PrintSupport Qt5::DBus)


install(TARGETS ${INSTANCE_NAME} RUNTIME DESTINATION ${GUI_DIR})
install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.boomaga.service
               ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service
               @ONLY)

include(cmake/CreateDesktopFile.cmake)
CREATE_DESKTOP_FILE(misc/boomaga.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/boomaga.desktop translations/*.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service DESTINATION ${DBUS_SERVICE_DIR})

install(FILES   images/boomaga_16.png   DESTINATION "share/icons/hicolor/16x16/apps"   RENAME "boomaga.png")
install(FILES   images/boomaga_32.png   DESTINATION "share/icons/hicolor/32x32/apps"   RENAME "boomaga.png")
install(FILES   images/boomaga_64.png   DESTINATION "share/icons/hicolor/64x64/apps"   RENAME "boomaga.png")
install(FILES   images/boomaga_128.png  DESTINATION "share/icons/hicolor/128x128/apps" RENAME "boomaga.png")
install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/boomaga.desktop DESTINATION "share/applications")
install(FILES   misc/boomaga.xml        DESTINATION "share/mime/packages")

add_subdirectory(pdfmerger)

add_tests("tests")