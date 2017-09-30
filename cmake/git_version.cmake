function(git_version GIT_BRANCH GIT_COMMIT_HASH)

    # Get the current working branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${GIT_BRANCH} ${branch} PARENT_SCOPE)

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE hash
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${GIT_COMMIT_HASH} ${hash} PARENT_SCOPE)

endfunction()
