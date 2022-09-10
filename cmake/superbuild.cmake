include(ExternalProject)

# We always define NDEBUG because some dependencies (chipmunk, flac) spam the stdout and stderr with debug info.
# We disable warnings for CMake, C and C++.
# We always use optimisation for the C libraries even in debug (but low opti level). In release we use speed optimisation.

# TODO: Pass down compiler flags and build type ?

set(GNU_FLAGS                "-DNDEBUG -w")
set(GNU_FLAGS_DEBUG          "-Og -g")
set(GNU_FLAGS_RELWITHDEBINFO "-O2 -g")
set(GNU_FLAGS_RELEASE        "-O3 -s")
set(GNU_FLAGS_MINSIZEREL     "-Os -s")

set(MSVC_FLAGS                "/DWIN32 /D_WINDOWS /DNDEBUG /W0 /MD")
set(MSVC_FLAGS_DEBUG          "/Od /RTC1 /DEBUG /Zi")
set(MSVC_FLAGS_RELWITHDEBINFO "/O1 /DEBUG /Zi")
set(MSVC_FLAGS_RELEASE        "/O2")
set(MSVC_FLAGS_MINSIZEREL     "/O1 /Ob1")

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

# Shared dependencies
include(cmake/superbuild_not_enough_standards.cmake)

if(CAPTAL_BUILD_APYRE)
    include(cmake/superbuild_sdl2.cmake)
endif()

if(CAPTAL_BUILD_TEPHRA)
    # Tephra has no dependency
endif()

if(CAPTAL_BUILD_SWELL)
    include(cmake/superbuild_portaudio.cmake)
    include(cmake/superbuild_ogg.cmake)
    include(cmake/superbuild_vorbis.cmake)
    include(cmake/superbuild_flac.cmake)
endif()

if(CAPTAL_BUILD_CAPTAL)
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
       "-DCAPTAL_SUPERBUILD:BOOL=OFF"
       "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
       "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/install"
       "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}"
       "-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}"
       "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
       "-DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}"
       "-DCMAKE_CXX_FLAGS_MINSIZEREL:STRING=${CMAKE_CXX_FLAGS_MINSIZEREL}"
       "-DCAPTAL_USE_LTO:BOOL=${CAPTAL_USE_LTO}"
       "-DCAPTAL_BUILD_FOUNDATION_EXAMPLES:BOOL=${CAPTAL_BUILD_FOUNDATION_EXAMPLES}"
       "-DCAPTAL_BUILD_FOUNDATION_TESTS:BOOL=${CAPTAL_BUILD_FOUNDATION_TESTS}"
       "-DCAPTAL_BUILD_APYRE:BOOL=${CAPTAL_BUILD_APYRE}"
       "-DCAPTAL_BUILD_APYRE_STATIC:BOOL=${CAPTAL_BUILD_APYRE_STATIC}"
       "-DCAPTAL_BUILD_APYRE_EXAMPLES:BOOL=${CAPTAL_BUILD_APYRE_EXAMPLES}"
       "-DCAPTAL_BUILD_APYRE_TESTS:BOOL=${CAPTAL_BUILD_APYRE_TESTS}"
       "-DCAPTAL_BUILD_TEPHRA:BOOL=${CAPTAL_BUILD_TEPHRA}"
       "-DCAPTAL_BUILD_TEPHRA_STATIC:BOOL=${CAPTAL_BUILD_TEPHRA_STATIC}"
       "-DCAPTAL_BUILD_TEPHRA_EXAMPLES:BOOL=${CAPTAL_BUILD_TEPHRA_EXAMPLES}"
       "-DCAPTAL_BUILD_TEPHRA_TESTS:BOOL=${CAPTAL_BUILD_TEPHRA_TESTS}"
       "-DCAPTAL_BUILD_SWELL:BOOL=${CAPTAL_BUILD_SWELL}"
       "-DCAPTAL_BUILD_SWELL_STATIC:BOOL=${CAPTAL_BUILD_SWELL_STATIC}"
       "-DCAPTAL_BUILD_SWELL_EXAMPLES:BOOL=${CAPTAL_BUILD_SWELL_EXAMPLES}"
       "-DCAPTAL_BUILD_SWELL_TESTS:BOOL=${CAPTAL_BUILD_SWELL_TESTS}"
       "-DCAPTAL_BUILD_CAPTAL:BOOL=${CAPTAL_BUILD_CAPTAL}"
       "-DCAPTAL_BUILD_CAPTAL_STATIC:BOOL=${CAPTAL_BUILD_CAPTAL_STATIC}"
       "-DCAPTAL_BUILD_CAPTAL_EXAMPLES:BOOL=${CAPTAL_BUILD_CAPTAL_EXAMPLES}"
       "-DCAPTAL_BUILD_CAPTAL_TESTS:BOOL=${CAPTAL_BUILD_CAPTAL_TESTS}"
       "-DTEPHRA_USE_PLATFORM_ANDROID:BOOL=${TEPHRA_USE_PLATFORM_ANDROID}"
       "-DTEPHRA_USE_PLATFORM_IOS:BOOL=${TEPHRA_USE_PLATFORM_IOS}"
       "-DTEPHRA_USE_PLATFORM_WIN32:BOOL=${TEPHRA_USE_PLATFORM_WIN32}"
       "-DTEPHRA_USE_PLATFORM_MACOS:BOOL=${TEPHRA_USE_PLATFORM_MACOS}"
       "-DTEPHRA_USE_PLATFORM_XLIB:BOOL=${TEPHRA_USE_PLATFORM_XLIB}"
       "-DTEPHRA_USE_PLATFORM_XCB:BOOL=${TEPHRA_USE_PLATFORM_XCB}"
       "-DTEPHRA_USE_PLATFORM_WAYLAND:BOOL=${TEPHRA_USE_PLATFORM_WAYLAND}"
       ${ADDITIONAL_CMAKE_ARGS}
   )

