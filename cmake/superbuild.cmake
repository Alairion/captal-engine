include(ExternalProject)

# We always define NDEBUG because some dependencies (chipmunk, flac) spam the stdout and stderr with debug info.
# We disable warnings for CMake, C and C++.
# We always use optimisation for the C libraries even in debug (but low opti level). In release we use speed optimisation.
macro(external_flags_gnu)
    set(EXTERNAL_FLAGS                "-DNDEBUG -w")
    set(EXTERNAL_FLAGS_DEBUG          "-Og -g")
    set(EXTERNAL_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(EXTERNAL_FLAGS_RELEASE        "-O3 -s")
    set(EXTERNAL_FLAGS_MINSIZEREL     "-Os -s")
endmacro()

macro(external_flags_msvc)
    set(EXTERNAL_FLAGS                "/DWIN32 /D_WINDOWS /DNDEBUG /W0 /MD")
    set(EXTERNAL_FLAGS_DEBUG          "/Od /RTC1 /DEBUG /Zi")
    set(EXTERNAL_FLAGS_RELWITHDEBINFO "/O1 /DEBUG /Zi")
    set(EXTERNAL_FLAGS_RELEASE        "/O2")
    set(EXTERNAL_FLAGS_MINSIZEREL     "/O1 /Ob1")
endmacro()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        external_flags_msvc()
    elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        external_flags_gnu()
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    external_flags_gnu()
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    external_flags_msvc()
endif()

set(DEPENDENCIES)
set(ADDITIONAL_CMAKE_ARGS)

# Foundation and shared dependencies
include(cmake/superbuild_not_enough_standards.cmake)

if(CPT_BUILD_APYRE)
    include(cmake/superbuild_sdl2.cmake)
endif()

#if(CPT_BUILD_TEPHRA)
#    # Tephra has no dependency to build
#endif()

if(CPT_BUILD_SWELL)
    include(cmake/superbuild_portaudio.cmake)
    include(cmake/superbuild_ogg.cmake)
    include(cmake/superbuild_vorbis.cmake)
    include(cmake/superbuild_flac.cmake)
endif()

if(CPT_BUILD_CAPTAL)
    include(cmake/superbuild_zlib.cmake)
    include(cmake/superbuild_freetype.cmake)
    include(cmake/superbuild_chipmunk.cmake)
    include(cmake/superbuild_pugixml.cmake)
    include(cmake/superbuild_sigslot.cmake)
    include(cmake/superbuild_fastfloat.cmake)
    include(cmake/superbuild_entt.cmake)
endif()

message(STATUS "Additional parameters: ")
foreach(param ${ADDITIONAL_CMAKE_ARGS})
    message(STATUS ${param})
endforeach()

ExternalProject_Add(Captal
    DEPENDS ${DEPENDENCIES}
    SOURCE_DIR ${PROJECT_SOURCE_DIR}
    CMAKE_CACHE_ARGS
        "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
        "-DCPT_SUPERBUILD:BOOL=OFF"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/install"
        "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}"
        "-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}"
        "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
        "-DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}"
        "-DCMAKE_CXX_FLAGS_MINSIZEREL:STRING=${CMAKE_CXX_FLAGS_MINSIZEREL}"
        "-DCPT_USE_LTO:BOOL=${CPT_USE_LTO}"
        "-DCPT_BUILD_FOUNDATION_EXAMPLES:BOOL=${CPT_BUILD_FOUNDATION_EXAMPLES}"
        "-DCPT_BUILD_FOUNDATION_TESTS:BOOL=${CPT_BUILD_FOUNDATION_TESTS}"
        "-DCPT_BUILD_APYRE:BOOL=${CPT_BUILD_APYRE}"
        "-DCPT_BUILD_APYRE_STATIC:BOOL=${CPT_BUILD_APYRE_STATIC}"
        "-DCPT_BUILD_APYRE_EXAMPLES:BOOL=${CPT_BUILD_APYRE_EXAMPLES}"
        "-DCPT_BUILD_APYRE_TESTS:BOOL=${CPT_BUILD_APYRE_TESTS}"
        "-DCPT_BUILD_TEPHRA:BOOL=${CPT_BUILD_TEPHRA}"
        "-DCPT_BUILD_TEPHRA_STATIC:BOOL=${CPT_BUILD_TEPHRA_STATIC}"
        "-DCPT_BUILD_TEPHRA_EXAMPLES:BOOL=${CPT_BUILD_TEPHRA_EXAMPLES}"
        "-DCPT_BUILD_TEPHRA_TESTS:BOOL=${CPT_BUILD_TEPHRA_TESTS}"
        "-DCPT_BUILD_SWELL:BOOL=${CPT_BUILD_SWELL}"
        "-DCPT_BUILD_SWELL_STATIC:BOOL=${CPT_BUILD_SWELL_STATIC}"
        "-DCPT_BUILD_SWELL_EXAMPLES:BOOL=${CPT_BUILD_SWELL_EXAMPLES}"
        "-DCPT_BUILD_SWELL_TESTS:BOOL=${CPT_BUILD_SWELL_TESTS}"
        "-DCPT_BUILD_CAPTAL:BOOL=${CPT_BUILD_CAPTAL}"
        "-DCPT_BUILD_CAPTAL_STATIC:BOOL=${CPT_BUILD_CAPTAL_STATIC}"
        "-DCPT_BUILD_CAPTAL_EXAMPLES:BOOL=${CPT_BUILD_CAPTAL_EXAMPLES}"
        "-DCPT_BUILD_CAPTAL_TESTS:BOOL=${CPT_BUILD_CAPTAL_TESTS}"
        ${ADDITIONAL_CMAKE_ARGS}
   )

