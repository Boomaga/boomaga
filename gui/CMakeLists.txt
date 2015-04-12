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

project(boomagaGui)

set(HEADERS
    kernel/sheet.h
    kernel/inputfile.h
    kernel/job.h
    kernel/tmppdffile.h
    kernel/layout.h
    kernel/render.h
    kernel/project.h
    kernel/printer.h
    boomagatypes.h
    mainwindow.h
    actions.h
    settings.h
    widgets.h
    dbus.h
    icon.h
    printersettings/printersettings.h
    previewwidget.h
    aboutdialog/aboutdialog.h
    translations/translatorsinfo/translatorsinfo.h
    export/exporttopdf.h
    export/exporttopdfprinter.h
    kernel/boomagapoppler.h
    kernel/projectfile.h
    pdfmerger/pdfmergeripc.h
    configdialog/configdialog.h
    printdialog/printdialog.h
)

set(SOURCES
    main.cpp
    kernel/sheet.cpp
    kernel/inputfile.cpp
    kernel/job.cpp
    kernel/tmppdffile.cpp
    kernel/layout.cpp
    kernel/render.cpp
    kernel/project.cpp
    kernel/printer.cpp
    boomagatypes.cpp
    mainwindow.cpp
    actions.cpp
    settings.cpp
    widgets.cpp
    dbus.cpp
    icon.cpp
    printersettings/printersettings.cpp
    previewwidget.cpp
    aboutdialog/aboutdialog.cpp
    translations/translatorsinfo/translatorsinfo.cpp
    export/exporttopdf.cpp
    export/exporttopdfprinter.cpp
    kernel/boomagapoppler.cpp
    kernel/projectfile.cpp
    pdfmerger/pdfmergeripc.cpp
    configdialog/configdialog.cpp
    printdialog/printdialog.cpp
)

set (MOCS
    kernel/project.h
    kernel/tmppdffile.h
    kernel/render.h

    settings.h
    mainwindow.h
    actions.h
    widgets.h
    dbus.h
    printersettings/printersettings.h
    previewwidget.h
    aboutdialog/aboutdialog.h
    export/exporttopdf.h
    kernel/projectfile.h
    pdfmerger/pdfmergeripc.h
    configdialog/configdialog.h
    printdialog/printdialog.h
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

option(USE_QT5 "Force use the Qt5." $ENV{USE_QT5})
option(USE_QT4 "Force use the Qt4." $ENV{USE_QT4})

if((USE_QT4 AND USE_QT5) OR
   (NOT USE_QT4 AND NOT USE_QT5))
    find_package(Qt4 QUIET)
    if (QT4_FOUND)
        set(USE_QT4 ON)
        set(USE_QT5 OFF)
    else()
        set(USE_QT4 OFF)
        set(USE_QT5 ON)
    endif()
endif()

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Cups REQUIRED QUIET)
find_package(snappy REQUIRED QUIET)
find_package(Poppler REQUIRED QUIET)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/libspectre
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CUPS_INCLUDE_DIR}
    ${SNAPPY_INCLUDE_DIRS}
    ${POPPLER_INCLUDE_DIRS}
    ${POPPLERCPP_INCLUDE_DIRS}
)

include(translations/translatorsinfo/CMakeLists.txt)
get_translatorsinfo_qrc(translatorsinfo_qrc)

if(USE_QT4)
    find_package(Qt4 REQUIRED QUIET)
    include(${QT_USE_FILE})
    qt4_wrap_cpp(MOC_SOURCES ${MOCS})
    qt4_add_resources(QRC_SOURCES ${RESOURCES})
    qt4_wrap_ui(UI_HEADERS ${UI_FILES})
    qt4_add_translation(QM_FILES ${TS_FILES})
    qt4_add_resources(QRC_CXX_SOURCES ${translatorsinfo_qrc})
else()
    cmake_minimum_required(VERSION 2.8.3)
    find_package(Qt5Widgets REQUIRED QUIET)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    add_definitions(${Qt5Widgets_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Widgets_LIBRARIES})

    find_package(Qt5PrintSupport REQUIRED QUIET)
    include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
    add_definitions(${Qt5PrintSupport_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5PrintSupport_LIBRARIES})

    find_package(Qt5DBus REQUIRED QUIET)
    include_directories(${Qt5DBus_INCLUDE_DIRS})
    add_definitions(${Qt5DBus_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5DBus_LIBRARIES})


    find_package(Qt5LinguistTools REQUIRED QUIET)


    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


    qt5_wrap_cpp(MOC_SOURCES ${MOCS})
    qt5_add_resources(QRC_SOURCES ${RESOURCES})
    qt5_wrap_ui(UI_HEADERS ${UI_FILES})
    qt5_add_translation(QM_FILES ${TS_FILES})
    qt5_add_resources(QRC_CXX_SOURCES ${translatorsinfo_qrc})

endif()
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${CUPS_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${SNAPPY_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${POPPLER_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${POPPLERCPP_LIBRARIES})

add_executable(${INSTANCE_NAME} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QM_FILES} ${QRC_SOURCES} ${UI_HEADERS} ${QRC_CXX_SOURCES})
target_link_libraries(${INSTANCE_NAME} ${LIBRARIES})


if(BUILD_TESTS STREQUAL "Yes")
    add_subdirectory(tests)
else()
    message(STATUS "")
    message(STATUS "For building tests use -DBUILD_TESTS=Yes option.")
    message(STATUS "")
endif()


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

if(USE_QT4)
    message(STATUS "Use Qt4, for building with Qt5 use -DUSE_QT5=Yes option.")
else()
    message(STATUS "Use Qt5, for building with Qt4 use -DUSE_QT4=Yes option.")
endif()
