cmake_minimum_required(VERSION 3.15.7)

project(captal_engine VERSION 0.1.0
                      DESCRIPTION "Captal Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

include(cmake/download_submodule.cmake)

if(CAPTAL_BUILD_FOUNDATION_TESTS OR CAPTAL_BUILD_APYRE_TESTS OR CAPTAL_BUILD_TEPHRA_TESTS OR CAPTAL_BUILD_SWELL_TESTS OR CAPTAL_BUILD_CAPTAL_TESTS)
    include(cmake/buildcatch.cmake)
endif()

include(cmake/buildnes.cmake)

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

#export(TARGETS captal_foundation apyre tephra swell captal NAMESPACE Captal:: FILE CaptalTargets.cmake)
