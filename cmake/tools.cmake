if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
    add_definitions("-g")
endif()

set(STATUS_MESSAGES "" CACHE INTERNAL "STATUS_MESSAGES_CACHE")

macro(statusMessage message)
        set(STATUS_MESSAGES ${STATUS_MESSAGES} ${message}  CACHE INTERNAL "STATUS_MESSAGES_CACHE")
endmacro()


macro(showStatus)
    message(STATUS "*****************************************************")

    foreach(msg ${STATUS_MESSAGES})
        message(STATUS "* ${msg}")
    endforeach()

    message(STATUS "*****************************************************")
endmacro()


macro(setByDefault VAR_NAME VAR_VALUE)
  if (NOT DEFINED ${VAR_NAME})
    set (${VAR_NAME} ${VAR_VALUE})
  endif()
  add_definitions(-D${VAR_NAME}=\"${VAR_VALUE}\")
endmacro()


macro(getQtVersion QT_VER)
    option(USE_QT5 "Force use the Qt5." $ENV{USE_QT5})
    option(USE_QT4 "Force use the Qt4." $ENV{USE_QT4})

    if((USE_QT4 AND USE_QT5) OR
       (NOT USE_QT4 AND NOT USE_QT5))
        find_package(Qt4 QUIET)

        if(QT4_FOUND)
            set(USE_QT4 ON)
            set(USE_QT5 OFF)
            set(${QT_VER} 4)

        else()
            set(USE_QT4 OFF)
            set(USE_QT5 ON)
            set(${QT_VER} 5)

        endif()
    endif()

    if(USE_QT4)
        statusMessage("Using Qt4, for building with Qt5 use -DUSE_QT5=Yes option.")
    else()
        statusMessage("Using Qt5, for building with Qt4 use -DUSE_QT4=Yes option.")
    endif()

endmacro()


macro(setFirstUpper VAR_NAME VAR_VALUE)
  string(SUBSTRING ${VAR_VALUE} 0 1 FIRST_LETTER)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
  string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" ${VAR_NAME} "${VAR_VALUE}")
endmacro()
