# Download and build external project PortAudio

option(CAPTAL_SUPERBUILD_EXCLUDE_PORTAUDIO "Does not build PortAudio as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_PORTAUDIO)
    set(CAPTAL_SUPERBUILD_PORTAUDIO_GIT_URL "https://github.com/PortAudio/portaudio.git" CACHE STRING "Used url for PortAudio git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_PORTAUDIO_GIT_TAG "v19.7.0" CACHE STRING "Used tag for PortAudio git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_PORTAUDIO_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_PORTAUDIO_GIT_TAG)

    ExternalProject_Add(PortAudio
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_PORTAUDIO_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_PORTAUDIO_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/portaudio"
        CMAKE_ARGS
            "-Wno-dev"

            "-DPA_BUILD_EXAMPLES=OFF"
            "-DPA_BUILD_SHARED=OFF"
            "-DPA_BUILD_STATIC=ON"
            "-DPA_BUILD_TESTS=OFF"
            "-DPA_ENABLE_DEBUG_OUTPUT=OFF"
            "-DPA_UNICODE_BUILD=ON"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/portaudio/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-Dportaudio_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/portaudio/install/lib/cmake/portaudio")
    list(APPEND DEPENDENCIES "PortAudio")
endif()
