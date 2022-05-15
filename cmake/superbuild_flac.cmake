# Download and build external project Flac

option(CAPTAL_SUPERBUILD_EXCLUDE_FLAC "Does not build Flac as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_FLAC)
    set(CAPTAL_SUPERBUILD_FLAC_GIT_URL "https://github.com/xiph/flac.git" CACHE STRING "Used url for Flac git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_FLAC_GIT_TAG "1.3.4" CACHE STRING "Used tag for Flac git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_FLAC_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_FLAC_GIT_TAG)

    ExternalProject_Add(Flac
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_FLAC_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_FLAC_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/flac"
        CMAKE_ARGS
            "-Wno-dev"

            "-DINSTALL_CMAKE_CONFIG_MODULE=ON"
            "-DBUILD_TESTING=OFF"
            "-DBUILD_EXAMPLES=OFF"
            "-DBUILD_PROGRAMS=OFF"
            "-DBUILD_SHARED_LIBS=OFF"
            "-DBUILD_CXXLIBS=OFF"
            "-DBUILD_DOCS=OFF"
            "-DWITH_STACK_PROTECTOR=OFF"
            "-DWITH_OGG=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/flac/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DFLAC_DIR=${CMAKE_BINARY_DIR}/dependencies/flac/install/share/FLAC/cmake")
    list(APPEND DEPENDENCIES "Flac")
endif()
