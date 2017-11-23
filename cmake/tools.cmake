file(GLOB cmakeFiles ${CMAKE_CURRENT_LIST_DIR}/*.cmake)
foreach(cmakeFile ${cmakeFiles})
    if ( NOT ${cmakeFile} STREQUAL ${CMAKE_CURRENT_LIST_FILE} )
        INCLUDE(${cmakeFile})
    endif()
endforeach(cmakeFile)




if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")

#if (CMAKE_BUILD_TYPE STREQUAL Release)
#  status_message("For building debug version use -DCMAKE_BUILD_TYPE=Debug option.")
#endif()

macro(setByDefault VAR_NAME VAR_VALUE)
  if (NOT DEFINED ${VAR_NAME})
    set (${VAR_NAME} ${VAR_VALUE})
  endif()
  add_definitions(-D${VAR_NAME}=\"${VAR_VALUE}\")
endmacro()



macro(setFirstUpper VAR_NAME VAR_VALUE)
  string(SUBSTRING ${VAR_VALUE} 0 1 FIRST_LETTER)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
  string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" ${VAR_NAME} "${VAR_VALUE}")
endmacro()



macro(add_tests TESTS_DIR)
  option(BUILD_TESTS "Build tests." $ENV{BUILD_TESTS})

  if(BUILD_TESTS)
    add_definitions(-DBUILD_TESTS)
    add_subdirectory(${TESTS_DIR})
  else()
    status_message("For building tests use -DBUILD_TESTS=Yes option.")
  endif()
endmacro()
