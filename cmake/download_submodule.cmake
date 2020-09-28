set(CAPTAL_ROOT_DIRECTORY ${PROJECT_SOURCE_DIR})

find_package(Git QUIET)

function(captal_download_submodule PATH CHECK_CMAKEFILES)

    if(CHECK_CMAKEFILES)
        if(EXISTS "${CAPTAL_ROOT_DIRECTORY}/${PATH}/CMakeLists.txt")
            message(STATUS "Submodule ${PATH} already downloaded")
            return()
        endif()
    else()
        if(EXISTS "${CAPTAL_ROOT_DIRECTORY}/${PATH}")
            message(STATUS "Submodule ${PATH} already downloaded")
            return()
        endif()
    endif()

    if(NOT GIT_FOUND)
        message(FATAL_ERROR "Git is not installed on this device, and is required to download submodules.")
    endif()

    message(STATUS "Downloading submodule ${PATH}")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init ${PATH}
                    WORKING_DIRECTORY ${CAPTAL_ROOT_DIRECTORY}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)

    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "\"git submodule update --init ${PATH}\" failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()

    if(CHECK_CMAKEFILES)
        if(NOT EXISTS "${CAPTAL_ROOT_DIRECTORY}/${PATH}/CMakeLists.txt")
            message(FATAL_ERROR "The submodule \"${PATH}\" has not been downloaded! Try update submodules and try again.")
        endif()
    else()
        if(NOT EXISTS "${CAPTAL_ROOT_DIRECTORY}/${PATH}")
            message(FATAL_ERROR "The submodule \"${PATH}\" has not been downloaded! Try update submodules and try again.")
        endif()
    endif()

    message(STATUS "Successfully downloaded submodule ${PATH}")

endfunction()
