include(ExternalProject)

# We always define NDEBUG because some dependencies (chipmunk, flac) spam the stdout and stderr with debug info.
# We disable warnings for CMake, C and C++.
# We always use optimisation for the C libraries even in debug (but low opti level). In release we use speed optimisation.

set(GNU_FLAGS                "-DNDEBUG -w")
set(GNU_FLAGS_DEBUG          "-Og -g")
set(GNU_FLAGS_RELWITHDEBINFO "-O2 -g")
set(GNU_FLAGS_RELEASE        "-O3 -s")
set(GNU_FLAGS_MINSIZEREL     "-Os -s")

set(MSVC_FLAGS                "/DWIN32 /D_WINDOWS /DNDEBUG /W0 /MD")
set(MSVC_FLAGS_DEBUG          "/RTCsu /O1 /DEBUG /Z7")
set(MSVC_FLAGS_RELWITHDEBINFO "/O1 /DEBUG /Z7")
set(MSVC_FLAGS_RELEASE        "/O2")
set(MSVC_FLAGS_MINSIZEREL     "/O1")

macro(external_flags_gnu)
    set(EXTERNAL_FLAGS                ${GNU_FLAGS})
    set(EXTERNAL_FLAGS_DEBUG          ${GNU_FLAGS_DEBUG})
    set(EXTERNAL_FLAGS_RELWITHDEBINFO ${GNU_FLAGS_RELWITHDEBINFO})
    set(EXTERNAL_FLAGS_RELEASE        ${GNU_FLAGS_RELEASE})
    set(EXTERNAL_FLAGS_MINSIZEREL     ${GNU_FLAGS_MINSIZEREL})
endmacro()

macro(external_flags_msvc)
    set(EXTERNAL_FLAGS                ${MSVC_FLAGS})
    set(EXTERNAL_FLAGS_DEBUG          ${MSVC_FLAGS_DEBUG})
    set(EXTERNAL_FLAGS_RELWITHDEBINFO ${MSVC_FLAGS_RELWITHDEBINFO})
    set(EXTERNAL_FLAGS_RELEASE        ${MSVC_FLAGS_RELEASE})
    set(EXTERNAL_FLAGS_MINSIZEREL     ${MSVC_FLAGS_MINSIZEREL})
endmacro()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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

include(cmake/superbuild_nes.cmake)

# Shared dependencies
include(cmake/superbuild_not_enough_standards.cmake)

if(CAPTAL_BUILD_APYRE)
    include(cmake/superbuild_sdl2.cmake)

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DCAPTAL_BUILD_APYRE=ON")
endif()

if(CAPTAL_BUILD_TEPHRA)
    # Tephra has no dependency
    list(APPEND ADDITIONAL_CMAKE_ARGS "-DCAPTAL_BUILD_TEPHRA=ON")
endif()

if(CAPTAL_BUILD_SWELL)
    include(cmake/superbuild_portaudio.cmake)
    include(cmake/superbuild_ogg.cmake)
    include(cmake/superbuild_vorbis.cmake)
    include(cmake/superbuild_flac.cmake)

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DCAPTAL_BUILD_SWELL=ON")
endif()

if(CAPTAL_BUILD_CAPTAL)

endif()

if(CAPTAL_BUILD_CAPTAL)

endif()

message(STATUS "Additional parameters: ${CMAKE_ADDITIONAL_ARGS}")

ExternalProject_Add(Captal
    DEPENDS ${DEPENDENCIES}
    SOURCE_DIR ${PROJECT_SOURCE_DIR}
    CMAKE_ARGS
       "-DCAPTAL_SUPERBUILD=OFF"
       "-DCAPTAL_USE_LTO=${CAPTAL_USE_LTO}"
       "-DCAPTAL_BUILD_FOUNDATION_EXAMPLES=${CAPTAL_BUILD_FOUNDATION_EXAMPLES}"
       "-DCAPTAL_BUILD_FOUNDATION_TESTS=${CAPTAL_BUILD_FOUNDATION_TESTS}"
       "-DCAPTAL_BUILD_APYRE=${CAPTAL_BUILD_APYRE}"
       "-DCAPTAL_BUILD_APYRE_STATIC=${CAPTAL_BUILD_APYRE_STATIC}"
       "-DCAPTAL_BUILD_APYRE_EXAMPLES=${CAPTAL_BUILD_APYRE_EXAMPLES}"
       "-DCAPTAL_BUILD_APYRE_TESTS=${CAPTAL_BUILD_APYRE_TESTS}"
       "-DCAPTAL_BUILD_TEPHRA=${CAPTAL_BUILD_TEPHRA}"
       "-DCAPTAL_BUILD_TEPHRA_STATIC=${CAPTAL_BUILD_TEPHRA_STATIC}"
       "-DCAPTAL_BUILD_TEPHRA_EXAMPLES=${CAPTAL_BUILD_TEPHRA_EXAMPLES}"
       "-DCAPTAL_BUILD_TEPHRA_TESTS=${CAPTAL_BUILD_TEPHRA_TESTS}"
       "-DCAPTAL_BUILD_SWELL=${CAPTAL_BUILD_SWELL}"
       "-DCAPTAL_BUILD_SWELL_STATIC=${CAPTAL_BUILD_SWELL_STATIC}"
       "-DCAPTAL_BUILD_SWELL_EXAMPLES=${CAPTAL_BUILD_SWELL_EXAMPLES}"
       "-DCAPTAL_BUILD_SWELL_TESTS=${CAPTAL_BUILD_SWELL_TESTS}"
       "-DCAPTAL_BUILD_CAPTAL=${CAPTAL_BUILD_CAPTAL}"
       "-DCAPTAL_BUILD_CAPTAL_STATIC=${CAPTAL_BUILD_CAPTAL_STATIC}"
       "-DCAPTAL_BUILD_CAPTAL_EXAMPLES=${CAPTAL_BUILD_CAPTAL_EXAMPLES}"
       "-DCAPTAL_BUILD_CAPTAL_TESTS=${CAPTAL_BUILD_CAPTAL_TESTS}"
       ${ADDITIONAL_CMAKE_ARGS})

