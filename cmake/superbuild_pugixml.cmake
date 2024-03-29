# Download and build external project Pugixml

option(CPT_SUPERBUILD_EXCLUDE_PUGIXML "Does not build pugixml as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CPT_SUPERBUILD_EXCLUDE_PUGIXML)
    set(CPT_SUPERBUILD_PUGIXML_GIT_URL "https://github.com/zeux/pugixml.git" CACHE STRING "Used url for pugixml git clone (allow usage of mirrors or interal repo)")
    set(CPT_SUPERBUILD_PUGIXML_GIT_TAG "master" CACHE STRING "Used tag for pugixml git clone")

    mark_as_advanced(CPT_SUPERBUILD_PUGIXML_GIT_URL)
    mark_as_advanced(CPT_SUPERBUILD_PUGIXML_GIT_TAG)

    ExternalProject_Add(pugixml
        GIT_REPOSITORY ${CPT_SUPERBUILD_PUGIXML_GIT_URL}
        GIT_TAG        ${CPT_SUPERBUILD_PUGIXML_GIT_TAG}
        GIT_SHALLOW    TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/pugixml"
        CMAKE_ARGS
            "-Wno-dev"

            "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
            "-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}"
            "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}"
            "-DCMAKE_CXX_FLAGS_MINSIZEREL=${CMAKE_CXX_FLAGS_MINSIZEREL}"

            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/pugixml/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-Dpugixml_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/pugixml/install/lib/cmake/pugixml")
    list(APPEND DEPENDENCIES "pugixml")
endif()
