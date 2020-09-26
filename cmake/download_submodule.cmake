function(captal_download_submodule PATH)

    find_package(Git QUIET)

    if(GIT_FOUND)
        message(STATUS "Downloading submodule ${PATH}")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init ${PATH}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PATH}/CMakeLists.txt")
        if(NOT GIT_FOUND)
            message(FATAL_ERROR "Git is not installed on this device, and is required to download submodules.")
        else()
            message(FATAL_ERROR "The submodule \"${PATH}\" has not been downloaded! Try update submodules and try again.")
        endif()
    endif()

endfunction()
