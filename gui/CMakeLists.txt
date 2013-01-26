cmake_minimum_required( VERSION 2.6 )

project(boomagaGui)

set(HEADERS
    kernel/psconstants.h
    kernel/psfile.h
    kernel/psproject.h
    kernel/pssheet.h
    kernel/psengine.h
    kernel/printer.h
    third-party/libspectre/ps.h
    third-party/libspectre/spectre-utils.h
    mainwindow.h
    settings.h
    widgets.h

    dbus.h
    printersettings/printersettings.h
    psrender.h
    psview.h
)

set(SOURCES
    main.cpp
    kernel/psfile.cpp
    kernel/psproject.cpp
    kernel/pssheet.cpp
    kernel/psengine.cpp
    kernel/printer.cpp
    third-party/libspectre/ps.c
    third-party/libspectre/spectre-utils.c
    mainwindow.cpp
    settings.cpp
    widgets.cpp
    dbus.cpp
    printersettings/printersettings.cpp
    psrender.cpp
    psview.cpp
)

set (MOCS
    kernel/psfile.h
    kernel/psproject.h
    mainwindow.h
    widgets.h
    dbus.h
    printersettings/printersettings.h
    psrender.h
    psview.h
)

set(LIBRARIES
)

set(QT_USE_QTDBUS 1)

set(UI_FILES
    mainwindow.ui
    printersettings/printersettings.ui
)

# QRC files .................
set(RESOURCES
    resource.qrc
)

# add translations ...
file (GLOB TS_FILES 
    translations/*.ts
)

set(TRANSLATIONS_DIR
    "${GUI_SHARE_DIR}/translations"
)
add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")


############################################
if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug)
#if(CMAKE_COMPILER_IS_GNUCXX)
#    add_definitions (-Wall -pedantic)
#endif()

#*******************************************

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Qt4 REQUIRED)
find_package(Ghostscript REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/libspectre
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GHOSTSCRIPT_INCLUDES}
)


include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_SOURCES ${MOCS})
qt4_add_resources(QRC_SOURCES ${RESOURCES})
qt4_wrap_ui(UI_HEADERS ${UI_FILES})
qt4_add_translation(QM_FILES ${TS_FILES})


add_executable(boomaga ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QM_FILES} ${QRC_SOURCES} ${UI_HEADERS})
target_link_libraries(boomaga ${LIBRARIES} ${QT_LIBRARIES} ${GHOSTSCRIPT_LIBRARIES})

install(TARGETS boomaga RUNTIME DESTINATION ${GUI_DIR})
install(FILES ${QM_FILES} DESTINATION ${TRANSLATIONS_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.boomaga.service
               ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service
               @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.boomaga.service DESTINATION ${DBUS_SERVICE_DIR})



