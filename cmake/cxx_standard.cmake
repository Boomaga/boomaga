# C++ standard version for CMake less 3.1
if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_STANDARD EQUAL 11)
        set (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    elseif (CMAKE_CXX_STANDARD EQUAL 14)
        set (CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    endif()
endif()
