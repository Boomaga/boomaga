set(STATUS_MESSAGES "" CACHE INTERNAL "STATUS_MESSAGES_CACHE")

macro(status_message message)
        set(STATUS_MESSAGES ${STATUS_MESSAGES} ${message}  CACHE INTERNAL "STATUS_MESSAGES_CACHE")
endmacro()


macro(show_status)
    message(STATUS "*****************************************************")

    foreach(msg ${STATUS_MESSAGES})
        message(STATUS "* ${msg}")
    endforeach()

    message(STATUS "*****************************************************")
endmacro()
