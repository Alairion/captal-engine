# Download and build external project Chipmunk

option(CAPTAL_SUPERBUILD_EXCLUDE_CHIPMUNK "Does not build Chipmunk as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_CHIPMUNK)
    set(CAPTAL_SUPERBUILD_CHIPMUNK_GIT_URL "https://github.com/slembcke/Chipmunk2D.git" CACHE STRING "Used url for Chipmunk git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_CHIPMUNK_GIT_TAG "Chipmunk-7.0.3" CACHE STRING "Used tag for Chipmunk git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_CHIPMUNK_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_CHIPMUNK_GIT_TAG)

    ExternalProject_Add(Chipmunk
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_CHIPMUNK_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_CHIPMUNK_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/chipmunk"
        CMAKE_ARGS
            "-Wno-dev"

            "-DLIB_INSTALL_DIR=OFF"
            "-DBIN_INSTALL_DIR=OFF"
            "-DBUILD_DEMOS=OFF"
            "-DINSTALL_DEMOS=OFF"
            "-DBUILD_SHARED=OFF"
            "-DBUILD_STATIC=ON "
            "-DINSTALL_STATIC=ON"
            "-DFORCE_CLANG_BLOCKS=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/chipmunk/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DChipmunk_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/chipmunk/install")
    list(APPEND DEPENDENCIES "Chipmunk")
endif()