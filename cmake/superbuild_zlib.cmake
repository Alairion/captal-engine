# Download and build external project Zlib

option(CAPTAL_SUPERBUILD_EXCLUDE_ZLIB "Does not build Zlib as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_ZLIB)
    set(CAPTAL_SUPERBUILD_ZLIB_GIT_URL "https://github.com/zlib-ng/zlib-ng.git" CACHE STRING "Used url for Zlib git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_ZLIB_GIT_TAG "v2.0.0-RC2" CACHE STRING "Used tag for Zlib git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_ZLIB_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_ZLIB_GIT_TAG)

    ExternalProject_Add(Zlib
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_ZLIB_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_ZLIB_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/zlib"
        CMAKE_ARGS
            "-Wno-dev"

            "-DBUILD_SHARED_LIBS=OFF"
            "-DZLIB_ENABLE_TESTS=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/zlib/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DZLIB_ROOT=${CMAKE_BINARY_DIR}/dependencies/zlib/install")
    list(APPEND DEPENDENCIES "Zlib")
endif()
