 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 #
 # Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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
 
 
 project(boomaga_test)
cmake_minimum_required( VERSION 2.6 )


set(TEST_HEADERS
    testboomaga.h

    ../boomagatypes.h
)

set(TEST_SOURCES
    testboomaga.cpp

    ../boomagatypes.cpp
)

set(TEST_MOCS
    testboomaga.h
)


add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\")

set(TEST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output/")
file(MAKE_DIRECTORY ${TEST_OUT_DIR})
add_definitions(-DTEST_OUT_DIR=\"${TEST_OUT_DIR}\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(QT_USE_QTTEST true)


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

if(USE_QT4)
    find_package(Qt4 REQUIRED QUIET)
    include(${QT_USE_FILE})
    qt4_wrap_cpp(TEST_MOC_SOURCES ${TEST_MOCS})
    qt4_add_resources(QRC_SOURCES ${RESOURCES})
    #qt4_wrap_ui(UI_HEADERS ${UI_FILES})
    #qt4_add_translation(QM_FILES ${TS_FILES})
    #qt4_add_resources(QRC_CXX_SOURCES ${translatorsinfo_qrc})
else()
    cmake_minimum_required(VERSION 2.8.3)
    find_package(Qt5Widgets REQUIRED QUIET)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    add_definitions(${Qt5Widgets_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Widgets_LIBRARIES})

    #find_package(Qt5PrintSupport REQUIRED QUIET)
    #include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
    #add_definitions(${Qt5PrintSupport_DEFINITIONS})
    #set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5PrintSupport_LIBRARIES})

    #find_package(Qt5DBus REQUIRED QUIET)
    #include_directories(${Qt5DBus_INCLUDE_DIRS})
    #add_definitions(${Qt5DBus_DEFINITIONS})
    #set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5DBus_LIBRARIES})


    #find_package(Qt5LinguistTools REQUIRED QUIET)


    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


    qt5_wrap_cpp(TEST_MOC_SOURCES ${TEST_MOCS})
    #qt5_add_resources(QRC_SOURCES ${RESOURCES})
    #qt5_wrap_ui(UI_HEADERS ${UI_FILES})
    #qt5_add_translation(QM_FILES ${TS_FILES})
    #qt5_add_resources(QRC_CXX_SOURCES ${translatorsinfo_qrc})

endif()


add_executable(${PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES} ${TEST_MOC_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES}  ${LIBRARIES})

#add_executable(${INSTANCE_NAME} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QM_FILES} ${QRC_SOURCES} ${UI_HEADERS} ${QRC_CXX_SOURCES})
#target_link_libraries(${INSTANCE_NAME} ${LIBRARIES} ${QT_LIBRARIES} ${CUPS_LIBRARIES} ${SNAPPY_LIBRARIES} ${POPPLER_LIBRARIES} ${POPPLERCPP_LIBRARIES})
