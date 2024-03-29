# Download and build external project Fast_float

option(CPT_SUPERBUILD_EXCLUDE_FASTFLOAT "Does not build fastfloat as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_FASTFLOAT)
    set(CPT_SUPERBUILD_FASTFLOAT_GIT_URL "https://github.com/fastfloat/fast_float.git" CACHE STRING "Used url for fastfloat git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_FASTFLOAT_GIT_TAG "v3.4.0" CACHE STRING "Used tag for fastfloat git clone")

    mark_as_advanced(CPT_SUPERBUILD_FASTFLOAT_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_FASTFLOAT_GIT_TAG)

    ExternalProject_Add(fastfloat
        GIT_REPOSITORY ${CPT_SUPERBUILD_FASTFLOAT_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_FASTFLOAT_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/fastfloat"
        CMAKE_ARGS
            "-Wno-dev"

            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/fastfloat/install"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DFastFloat_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/fastfloat/install/share/FastFloat")
    list(APPEND DEPENDENCIES "fastfloat")
endif()
