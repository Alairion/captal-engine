# Download and build external project Not Enough Standards

option(CPT_SUPERBUILD_EXCLUDE_NOT_ENOUGH_STANDARDS "Does not build Not Enough Standards as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_NOT_ENOUGH_STANDARDS)
    set(CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_URL "https://github.com/Alairion/not-enough-standards.git" CACHE STRING "Used url for Not Enough Standards git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_TAG "v1.0.3" CACHE STRING "Used tag for NES git clone")

    mark_as_advanced(CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_TAG)

    ExternalProject_Add(NotEnoughStandards
        GIT_REPOSITORY ${CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_NOT_ENOUGH_STANDARDS_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/NotEnoughStandards"
        CMAKE_ARGS
            "-Wno-dev"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/nes/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DNotEnoughStandards_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/nes/install/lib/cmake/NotEnoughStandards")
    list(APPEND DEPENDENCIES "NotEnoughStandards")
endif()
