cmake_minimum_required(VERSION 3.15.7)

project(captal_engine VERSION 0.1.0
                      DESCRIPTION "Captal Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CAPTAL_BUILD_FOUNDATION_EXAMPLES  OFF "Build Captal Foundation's examples if ON")
option(CAPTAL_BUILD_FOUNDATION_TESTS     OFF "Build Captal Foundation's unit tests if ON")
option(CAPTAL_BUILD_APYRE                OFF "Build Apyre if ON")
option(CAPTAL_BUILD_APYRE_EXAMPLES       OFF "Build Apyre's examples if ON, does nothing if CAPTAL_BUILD_APYRE is off")
option(CAPTAL_BUILD_APYRE_TESTS          OFF "Build Apyre's unit tests if ON, does nothing if CAPTAL_BUILD_APYRE is off")
option(CAPTAL_BUILD_TEPHRA               OFF "Build Tephra if ON")
option(CAPTAL_BUILD_TEPHRA_EXAMPLES      OFF "Build Tephra's examples if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off")
option(CAPTAL_BUILD_TEPHRA_TESTS         OFF "Build Tephra's unit tests if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off")
option(CAPTAL_BUILD_SWELL                OFF "Build Swell if ON")
option(CAPTAL_BUILD_SWELL_EXAMPLES       OFF "Build Swell's examples if ON. Does nothing if CAPTAL_BUILD_SWELL is off")
option(CAPTAL_BUILD_SWELL_TESTS          OFF "Build Swell's unit tests if ON. Does nothing if CAPTAL_BUILD_SWELL is off")
option(CAPTAL_BUILD_CAPTAL               OFF "Build Captal if ON, implies CAPTAL_BUILD_TEPHRA, CAPTAL_BUILD_APYRE and CAPTAL_BUILD_SWELL if ON")
option(CAPTAL_BUILD_CAPTAL_EXAMPLES      OFF "Build Captal's examples if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off")
option(CAPTAL_BUILD_CAPTAL_TESTS         OFF "Build Captal's unit tests if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off")

if(CAPTAL_BUILD_CAPTAL)
    set(CAPTAL_BUILD_APYRE  ON CACHE BOOL "Build Apyre if ON"  FORCE)
    set(CAPTAL_BUILD_TEPHRA ON CACHE BOOL "Build Tephra if ON" FORCE)
    set(CAPTAL_BUILD_SWELL  ON CACHE BOOL "Build Swell if ON"  FORCE)
endif()

if(CAPTAL_BUILD_TEPHRA AND (CAPTAL_BUILD_TEPHRA_EXAMPLES OR CAPTAL_BUILD_TEPHRA_TESTS))
    set(CAPTAL_BUILD_APYRE ON CACHE BOOL "Build Apyre if ON"  FORCE)
endif()

include(cmake/download_submodule.cmake)

if(CAPTAL_BUILD_FOUNDATION_TESTS OR CAPTAL_BUILD_APYRE_TESTS OR CAPTAL_BUILD_TEPHRA_TESTS OR CAPTAL_BUILD_SWELL_TESTS OR CAPTAL_BUILD_CAPTAL_TESTS)
    include(cmake/buildcatch.cmake)
endif()

include(cmake/buildnes.cmake)

add_subdirectory(captal_foundation)
add_subdirectory(apyre)
add_subdirectory(tephra)
add_subdirectory(swell)
add_subdirectory(captal)
