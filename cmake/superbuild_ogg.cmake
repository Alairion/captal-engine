# Download and build external project Ogg

option(CPT_SUPERBUILD_EXCLUDE_OGG "Does not build Ogg as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_OGG)
    set(CPT_SUPERBUILD_OGG_GIT_URL "https://github.com/xiph/ogg.git" CACHE STRING "Used url for Ogg git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_OGG_GIT_TAG "v1.3.5" CACHE STRING "Used tag for Ogg git clone")

    mark_as_advanced(CPT_SUPERBUILD_OGG_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_OGG_GIT_TAG)

    ExternalProject_Add(Ogg
        GIT_REPOSITORY ${CPT_SUPERBUILD_OGG_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_OGG_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/ogg"
        CMAKE_ARGS
            "-Wno-dev"

            "-DINSTALL_CMAKE_PACKAGE_MODULE=ON"
            "-DINSTALL_PKG_CONFIG_MODULE=OFF"
            "-DINSTALL_DOCS=OFF"
            "-DBUILD_TESTING=OFF"
            "-DBUILD_FRAMEWORK=OFF"
            "-DBUILD_SHARED_LIBS=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/ogg/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DOgg_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/ogg/install/lib/cmake/Ogg")
    list(APPEND DEPENDENCIES "Ogg")
endif()
