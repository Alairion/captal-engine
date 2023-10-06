# Download and build external project Freetype

option(CPT_SUPERBUILD_EXCLUDE_FREETYPE "Does not build Freetype as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_FREETYPE)
    set(CPT_SUPERBUILD_FREETYPE_GIT_URL "https://gitlab.freedesktop.org/freetype/freetype" CACHE STRING "Used url for Freetype git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_FREETYPE_GIT_TAG "VER-2-12-1" CACHE STRING "Used tag for Freetype git clone")

    mark_as_advanced(CPT_SUPERBUILD_FREETYPE_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_FREETYPE_GIT_TAG)

    ExternalProject_Add(Freetype
        GIT_REPOSITORY ${CPT_SUPERBUILD_FREETYPE_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_FREETYPE_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/freetype"
        DEPENDS Zlib
        CMAKE_ARGS
            "-Wno-dev"

            "-DZLIB_ROOT=${CMAKE_BINARY_DIR}/dependencies/zlib/install"

            "-DBUILD_SHARED_LIBS=OFF"
            "-DFT_WITH_ZLIB=OFF"
            "-DFT_WITH_BZIP2=OFF"
            "-DFT_WITH_PNG=OFF"
            "-DFT_WITH_HARFBUZZ=OFF"
            "-DFT_WITH_BROTLI=OFF"
            "-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=OFF"
            "-DCMAKE_DISABLE_FIND_PACKAGE_BZip2=OFF"
            "-DCMAKE_DISABLE_FIND_PACKAGE_PNG=OFF"
            "-DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=OFF"
            "-DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/freetype/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DFreetype_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/freetype/install/lib/cmake/freetype")
    list(APPEND ADDITIONAL_CMAKE_ARGS "-DCAPTAL_INTERNAL_FREETYPE:BOOL=ON")
    list(APPEND DEPENDENCIES "Freetype")
endif()
