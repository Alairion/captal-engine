# Download and build external project Vorbis

option(CPT_SUPERBUILD_EXCLUDE_VORBIS "Does not build Vorbis as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_VORBIS)
    set(CPT_SUPERBUILD_VORBIS_GIT_URL "https://github.com/xiph/vorbis.git" CACHE STRING "Used url for Vorbis git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_VORBIS_GIT_TAG "v1.3.7" CACHE STRING "Used tag for Vorbis git clone")

    mark_as_advanced(CPT_SUPERBUILD_VORBIS_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_VORBIS_GIT_TAG)

    ExternalProject_Add(Vorbis
        GIT_REPOSITORY ${CPT_SUPERBUILD_VORBIS_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_VORBIS_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/vorbis"
        DEPENDS Ogg
        CMAKE_ARGS
            "-Wno-dev"

            "-DINSTALL_CMAKE_PACKAGE_MODULE=ON"
            "-DINSTALL_PKG_CONFIG_MODULE=OFF"
            "-DINSTALL_DOCS=OFF"
            "-DBUILD_TESTING=OFF"
            "-DBUILD_FRAMEWORK=OFF"
            "-DBUILD_SHARED_LIBS=OFF"
            "-DOGG_ROOT=${CMAKE_BINARY_DIR}/dependencies/ogg/install"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/vorbis/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DVorbis_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/vorbis/install/lib/cmake/Vorbis")
    list(APPEND DEPENDENCIES "Vorbis")
endif()
