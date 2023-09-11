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


cmake_minimum_required(VERSION 3.25.0)
cmake_policy(SET CMP0028 NEW)
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()


project(boomagaGui)

set(HEADERS
    boomagatypes.h
    dbus.h
    render.h
    settings.h
    finddbusaddress.h
    ../common.h

    kernel/sheet.h
    kernel/job.h
    kernel/tmppdffile.h
    kernel/layout.h
    kernel/project.h
    kernel/projectpage.h
    kernel/printer.h
    kernel/cupsprinteroptions.h
    kernel/pdfprocessor.h

    iofiles/infile.h
    iofiles/boofile.h
    iofiles/pdffile.h
    iofiles/cupsboofile.h
    iofiles/postscriptfile.h

    gui/mainwindow.h
    gui/actions.h
    gui/icon.h
    gui/aboutdialog/aboutdialog.h
    gui/configdialog/configdialog.h
    gui/export/exporttopdf.h
    gui/printdialog/printdialog.h
    gui/printersettings/printersettings.h
    gui/widgets/pagelistview.h
    gui/widgets/joblistview.h
    gui/widgets/layoutradiobutton.h
    gui/widgets/printerscombobox.h
    gui/widgets/previewwidget.h
    gui/widgets/subbookletview.h
    
    translations/translatorsinfo/translatorsinfo.h
    
    pdfparser/pdferrors.h
    pdfparser/pdfobject.h
    pdfparser/pdfreader.h
    pdfparser/pdfvalue.h
    pdfparser/pdfwriter.h
    pdfparser/pdfxref.h

)

set(SOURCES
    main.cpp
    boomagatypes.cpp
    dbus.cpp
    render.cpp
    settings.cpp
    finddbusaddress.cpp
    ../common.cpp

    kernel/sheet.cpp
    kernel/job.cpp
    kernel/tmppdffile.cpp
    kernel/layout.cpp
    kernel/project.cpp
    kernel/projectpage.cpp
    kernel/cupsprinteroptions.cpp
    kernel/printer.cpp
    kernel/pdfprocessor.cpp
    
    iofiles/infile.cpp
    iofiles/pdffile.cpp
    iofiles/boofile.cpp
    iofiles/cupsboofile.cpp
    iofiles/postscriptfile.cpp

    gui/mainwindow.cpp
    gui/actions.cpp
    gui/icon.cpp
    gui/aboutdialog/aboutdialog.cpp
    gui/configdialog/configdialog.cpp
    gui/export/exporttopdf.cpp
    gui/printdialog/printdialog.cpp
    gui/printersettings/printersettings.cpp
    gui/widgets/pagelistview.cpp
    gui/widgets/joblistview.cpp
    gui/widgets/layoutradiobutton.cpp
    gui/widgets/printerscombobox.cpp
    gui/widgets/previewwidget.cpp
    gui/widgets/subbookletview.cpp
    
    translations/translatorsinfo/translatorsinfo.cpp
    
    pdfparser/pdfobject.cpp
    pdfparser/pdfreader.cpp
    pdfparser/pdfvalue.cpp
    pdfparser/pdfwriter.cpp
    pdfparser/pdfxref.cpp
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
    misc/images/icons.qrc
    misc/images/mainicon.qrc
)

# add translations ...
file (GLOB TS_FILES
    translations/boomaga*.ts
)

set(TRANSLATIONS_DIR
    "${GUI_SHARE_DIR}/translations"
)
add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")


#*******************************************
include("${CMAKE_SOURCE_DIR}/cmake/macos.cmake")
add_homebrew_qt_prefix_path()

include("${CMAKE_SOURCE_DIR}/cmake/git_version.cmake")

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
include_directories(${POPPLERCPP_INCLUDE_DIRS})
set(LIBRARIES ${LIBRARIES} ${POPPLERCPP_LIBRARIES})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARIES})

if (APPLE)
 
    if (MAC_BUNDLE)
        add_definitions(-DMAC_BUNDLE=1)
        
        add_definitions(-DMAC_UPDATER=1)
        add_subdirectory(updater)
        set(LIBRARIES ${LIBRARIES} updater_mac)
    endif() 
endif()

include(translations/translatorsinfo/CMakeLists.txt)
get_translatorsinfo_qrc(translatorsinfo_qrc)
qt5_add_resources(TRANSLATORS_INFO_QRC ${translatorsinfo_qrc})

qt5_add_resources(  QRC_SOURCES ${RESOURCES})
qt5_add_translation(QM_FILES    ${TS_FILES})

add_executable(boomaga ${HEADERS} ${SOURCES} ${QM_FILES} ${QRC_SOURCES} ${TRANSLATORS_INFO_QRC})
target_link_libraries(boomaga ${LIBRARIES} Qt5::Widgets Qt5::PrintSupport Qt5::DBus)
set_target_properties(boomaga PROPERTIES OUTPUT_NAME "boomaga")




# MacOS ***************************************************
if( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

    install(TARGETS boomaga RUNTIME DESTINATION "Applications/Boomaga.app/Contents/MacOS")
    set_target_properties(boomaga PROPERTIES OUTPUT_NAME "Boomaga")    

    install(FILES misc/images/mainicon/Boomaga.icns DESTINATION "Applications/Boomaga.app/Contents/Resources")

    install(FILES ${QM_FILES} DESTINATION "Applications/Boomaga.app/Contents/translations")

    CREATE_PLIST_FILE(misc/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist DESTINATION "Applications/Boomaga.app/Contents")


# Linux + BSD ********************************************
else()
    install(TARGETS boomaga RUNTIME DESTINATION ${GUI_DIR})
    install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_DIR})

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.boomaga.service
                   ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service
                   @ONLY)

    include(cmake/CreateDesktopFile.cmake)
    CREATE_DESKTOP_FILE(misc/boomaga.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/boomaga.desktop translations/*.desktop)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service DESTINATION ${DBUS_SERVICE_DIR})


    install(FILES   misc/images/mainicon/boomaga-16x16.png   DESTINATION "share/icons/hicolor/16x16/apps"   RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-32x32.png   DESTINATION "share/icons/hicolor/32x32/apps"   RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-48x48.png   DESTINATION "share/icons/hicolor/48x48/apps"   RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-64x64.png   DESTINATION "share/icons/hicolor/64x64/apps"   RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-128x128.png DESTINATION "share/icons/hicolor/128x128/apps" RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-256x256.png DESTINATION "share/icons/hicolor/256x256/apps" RENAME "boomaga.png")
    install(FILES   misc/images/mainicon/boomaga-512x512.png DESTINATION "share/icons/hicolor/512x512/apps" RENAME "boomaga.png")

    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/boomaga.desktop DESTINATION "share/applications")
    install(FILES   misc/boomaga.xml        DESTINATION "share/mime/packages")
    install(FILES   misc/boomaga.1.gz       DESTINATION share/man/man1)
    # Linux + BSD *****************************************
endif()

add_tests("tests")
