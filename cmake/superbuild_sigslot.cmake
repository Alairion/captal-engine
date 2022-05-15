# Download and build external project sigslot

option(CAPTAL_SUPERBUILD_EXCLUDE_SIGSLOT "Does not build sigslot as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_SIGSLOT)
    set(CAPTAL_SUPERBUILD_SIGSLOT_GIT_URL "https://github.com/palacaze/sigslot.git" CACHE STRING "Used url for sigslot git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_SIGSLOT_GIT_TAG "v1.2.1" CACHE STRING "Used tag for sigslot git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_SIGSLOT_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_SIGSLOT_GIT_TAG)

    ExternalProject_Add(sigslot
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_SIGSLOT_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_SIGSLOT_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/sigslot"
        CMAKE_ARGS
            "-Wno-dev"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/sigslot/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DPalSigslot_DIR=${CMAKE_BINARY_DIR}/dependencies/sigslot/install/lib/cmake/PalSigslot")
    list(APPEND DEPENDENCIES "sigslot")
endif()
