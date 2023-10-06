# Download and build external project entt

option(CPT_SUPERBUILD_EXCLUDE_ENTT "Does not build entt as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_ENTT)
    set(CPT_SUPERBUILD_ENTT_GIT_URL "https://github.com/skypjack/entt.git" CACHE STRING "Used url for entt git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_ENTT_GIT_TAG "v3.10.1" CACHE STRING "Used tag for entt git clone")

    mark_as_advanced(CPT_SUPERBUILD_ENTT_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_ENTT_GIT_TAG)

    ExternalProject_Add(EnTT
        GIT_REPOSITORY ${CPT_SUPERBUILD_ENTT_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_ENTT_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/entt"
        CMAKE_ARGS
            "-Wno-dev"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/entt/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DEnTT_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/entt/install/lib/EnTT/cmake")
    list(APPEND DEPENDENCIES "EnTT")
endif()
