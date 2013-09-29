cmake_minimum_required( VERSION 2.6 )

project(boomagamerger)

set(MERGER_HEADERS
    pdfmerger.h
)

set(MERGER_SOURCES
    main.cpp
    pdfmerger.cpp
)

set(MERGER_MOCS
    #pdfmerger.h
)



#*******************************************
#message("CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
#message("EXECUTABLE_OUTPUT_PATH: ${PROJECT_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/..")
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#find_package(Qt4 REQUIRED QUIET)
#find_package(Poppler REQUIRED QUIET)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CUPS_INCLUDE_DIR}
    ${POPPLER_INCLUDE_DIRS}
)

include(${QT_USE_FILE})
qt4_wrap_cpp(MERGER_MOC_SOURCES ${MERGER_MOCS})

add_executable(${PROJECT_NAME} ${MERGER_HEADERS} ${MERGER_SOURCES} ${MERGER_MOC_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${POPPLER_LIBRARIES})

#install(TARGETS boomaga RUNTIME DESTINATION ${GUI_DIR})



