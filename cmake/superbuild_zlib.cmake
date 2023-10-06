# Download and build external project Zlib

option(CPT_SUPERBUILD_EXCLUDE_ZLIB "Does not build Zlib as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_ZLIB)
    set(CPT_SUPERBUILD_ZLIB_GIT_URL "https://github.com/zlib-ng/zlib-ng.git" CACHE STRING "Used url for Zlib git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_ZLIB_GIT_TAG "v2.0.0-RC2" CACHE STRING "Used tag for Zlib git clone")

    mark_as_advanced(CPT_SUPERBUILD_ZLIB_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_ZLIB_GIT_TAG)

    ExternalProject_Add(Zlib
        GIT_REPOSITORY ${CPT_SUPERBUILD_ZLIB_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_ZLIB_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/zlib"
        CMAKE_ARGS
            "-Wno-dev"

            "-DBUILD_SHARED_LIBS=OFF"
            "-DZLIB_ENABLE_TESTS=OFF"
            "-DZLIB_COMPAT=ON"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/zlib/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DZLIB_ROOT:PATH=${CMAKE_BINARY_DIR}/dependencies/zlib/install")
    list(APPEND DEPENDENCIES "Zlib")
endif()
