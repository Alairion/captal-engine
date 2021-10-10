cmake_minimum_required(VERSION 3.15.7)

project(captal_engine VERSION 0.1.0
                      DESCRIPTION "Captal Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

option(CAPTAL_USE_LTO "Build Captal and its submodules with LTO enabled, if supported" OFF)
option(CAPTAL_USE_CUSTOM_C_FLAGS "Build Captal and its submodules with predefined compiler options, if supported." ON)

option(CAPTAL_BUILD_FOUNDATION_EXAMPLES "Build Captal Foundation's examples if ON" OFF)
option(CAPTAL_BUILD_FOUNDATION_TESTS "Build Captal Foundation's unit tests if ON" OFF)

option(CAPTAL_BUILD_APYRE "Build Apyre if ON" OFF)
option(CAPTAL_BUILD_APYRE_STATIC "Build Apyre as a static library if ON" OFF)
option(CAPTAL_BUILD_APYRE_EXAMPLES "Build Apyre's examples if ON, does nothing if CAPTAL_BUILD_APYRE is off" OFF)
option(CAPTAL_BUILD_APYRE_TESTS "Build Apyre's unit tests if ON, does nothing if CAPTAL_BUILD_APYRE is off" OFF)

option(CAPTAL_BUILD_TEPHRA "Build Tephra if ON" OFF)
option(CAPTAL_BUILD_TEPHRA_STATIC "Build Tephra as a static library if ON" OFF)
option(CAPTAL_BUILD_TEPHRA_EXAMPLES "Build Tephra's examples if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off" OFF)
option(CAPTAL_BUILD_TEPHRA_TESTS "Build Tephra's unit tests if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off" OFF)

option(CAPTAL_BUILD_SWELL "Build Swell if ON" OFF)
option(CAPTAL_BUILD_SWELL_STATIC "Build Swell as a static library if ON" OFF)
option(CAPTAL_BUILD_SWELL_EXAMPLES "Build Swell's examples if ON. Does nothing if CAPTAL_BUILD_SWELL is off" OFF)
option(CAPTAL_BUILD_SWELL_TESTS "Build Swell's unit tests if ON. Does nothing if CAPTAL_BUILD_SWELL is off" OFF)

option(CAPTAL_BUILD_CAPTAL "Build Captal if ON, implies CAPTAL_BUILD_TEPHRA, CAPTAL_BUILD_APYRE and CAPTAL_BUILD_SWELL if ON" OFF)
option(CAPTAL_BUILD_CAPTAL_STATIC "Build Captal as a static library if ON" OFF)
option(CAPTAL_BUILD_CAPTAL_EXAMPLES "Build Captal's examples if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off" OFF)
option(CAPTAL_BUILD_CAPTAL_TESTS "Build Captal's unit tests if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off" OFF)

if(CAPTAL_BUILD_CAPTAL)
    set(CAPTAL_BUILD_APYRE  ON CACHE BOOL "Build Apyre if ON"  FORCE)
    set(CAPTAL_BUILD_TEPHRA ON CACHE BOOL "Build Tephra if ON" FORCE)
    set(CAPTAL_BUILD_SWELL  ON CACHE BOOL "Build Swell if ON"  FORCE)
endif()

if(CAPTAL_BUILD_TEPHRA AND (CAPTAL_BUILD_TEPHRA_EXAMPLES OR CAPTAL_BUILD_TEPHRA_TESTS))
    set(CAPTAL_BUILD_APYRE ON CACHE BOOL "Build Apyre if ON" FORCE)
endif()

if(CAPTAL_USE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORT OUTPUT LTO_OUTPUT)

    if(LTO_SUPPORT)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "" FORCE)
        message("LTO is enabled.")
    else()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
        message("LTO is not supported: ${LTO_OUTPUT}")
    endif()
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
endif()

if(CAPTAL_USE_CUSTOM_C_FLAGS)
    # We always define NDEBUG because some dependencies (chipmunk, flac) spam the stdout and stderr with debug info.
    # We always use optimisation for the C libraries even in debug. In release we use speed optimisation.

    set(GCC_CLANG_FLAGS_DEBUG "-DNDEBUG -Og -g")
    set(GCC_CLANG_FLAGS_RELEASE "-DNDEBUG -O3 -s")
    set(MSVC_FLAGS_DEBUG "/DNDEBUG /O1 /DEBUG")
    set(MSVC_FLAGS_RELEASE "/DNDEBUG /O2")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${MSVC_FLAGS_DEBUG}")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} ${MSVC_FLAGS_RELEASE}")
        elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GCC_CLANG_FLAGS_DEBUG}")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} ${GCC_CLANG_FLAGS_RELEASE}")
        endif()
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GCC_CLANG_FLAGS_DEBUG}")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} ${GCC_CLANG_FLAGS_RELEASE}")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${MSVC_FLAGS_DEBUG}")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} ${MSVC_FLAGS_RELEASE}")
    endif()
endif()

include(cmake/download_submodule.cmake)

if(CAPTAL_BUILD_FOUNDATION_TESTS OR CAPTAL_BUILD_APYRE_TESTS OR CAPTAL_BUILD_TEPHRA_TESTS OR CAPTAL_BUILD_SWELL_TESTS OR CAPTAL_BUILD_CAPTAL_TESTS)
    include(cmake/buildcatch.cmake)
endif()

include(cmake/buildnes.cmake)

set(TO_EXPORT "")

add_subdirectory(captal_foundation)

if(CAPTAL_BUILD_APYRE)
    add_subdirectory(apyre)
endif()

if(CAPTAL_BUILD_TEPHRA)
    add_subdirectory(tephra)
endif()

if(CAPTAL_BUILD_SWELL)
    add_subdirectory(swell)
endif()

if(CAPTAL_BUILD_CAPTAL)
    add_subdirectory(captal)
endif()

list(REMOVE_DUPLICATES TO_EXPORT)
message("Exporting: ${TO_EXPORT}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    export(TARGETS captal_foundation ${TO_EXPORT}
           NAMESPACE captal::
           FILE ${PROJECT_SOURCE_DIR}/captal_debug.cmake)
else()
    export(TARGETS captal_foundation ${TO_EXPORT}
           NAMESPACE captal::
           FILE ${PROJECT_SOURCE_DIR}/captal_release.cmake)
endif()
