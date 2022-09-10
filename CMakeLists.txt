cmake_minimum_required(VERSION 3.21.0)

project(captal_engine VERSION 0.1.0
                      DESCRIPTION "Captal Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CAPTAL_SUPERBUILD "Build all Captal's dependencies" ON)
option(CAPTAL_USE_LTO "Build Captal and its submodules with LTO enabled, if supported" OFF)

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

if(CAPTAL_BUILD_TEPHRA)
    include(CMakeDependentOption)

    cmake_dependent_option(TEPHRA_USE_PLATFORM_ANDROID "If on, enable the Android surface extension" ON "${CMAKE_SYSTEM_NAME} STREQUAL Android" OFF)
    cmake_dependent_option(TEPHRA_USE_PLATFORM_IOS "If on, enable the IOS surface extension" ON "${CMAKE_SYSTEM_NAME} STREQUAL iOS" OFF)
    cmake_dependent_option(TEPHRA_USE_PLATFORM_WIN32 "If on, enable the Windows surface extension" ON "${CMAKE_SYSTEM_NAME} STREQUAL Windows" OFF)
    cmake_dependent_option(TEPHRA_USE_PLATFORM_MACOS "If on, enable the OSX surface extension" ON "${CMAKE_SYSTEM_NAME} STREQUAL Darwin" OFF)
    cmake_dependent_option(TEPHRA_USE_PLATFORM_XLIB "If on, enable the Linux Xlib surface extension" ON "${CMAKE_SYSTEM_NAME} STREQUAL Linux" OFF)

    #Other linux platforms
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        option(TEPHRA_USE_PLATFORM_XCB "If on, enable the Linux XCB surface extension" OFF)
        option(TEPHRA_USE_PLATFORM_WAYLAND "If on, enable the Linux Wayland surface extension" OFF)
    endif()
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

if(CAPTAL_SUPERBUILD)
    include(cmake/superbuild.cmake)
    return()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

if(CAPTAL_BUILD_FOUNDATION_TESTS OR CAPTAL_BUILD_APYRE_TESTS OR CAPTAL_BUILD_TEPHRA_TESTS OR CAPTAL_BUILD_SWELL_TESTS OR CAPTAL_BUILD_CAPTAL_TESTS)
    include(cmake/download_submodule.cmake)
    include(cmake/buildcatch.cmake)
endif()


find_package(NotEnoughStandards REQUIRED)

add_subdirectory(captal_foundation)
set(TO_EXPORT CaptalFoundation)

if(CAPTAL_BUILD_APYRE)
    add_subdirectory(apyre)
    list(APPEND TO_EXPORT Apyre)
endif()

if(CAPTAL_BUILD_TEPHRA)
    add_subdirectory(tephra)
    list(APPEND TO_EXPORT Tephra)
endif()

if(CAPTAL_BUILD_SWELL)
    add_subdirectory(swell)
    list(APPEND TO_EXPORT Swell)
endif()

if(CAPTAL_BUILD_CAPTAL)
    add_subdirectory(captal)
    list(APPEND TO_EXPORT Captal)
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/CaptalConfig.cmake.in
    ${PROJECT_BINARY_DIR}/CaptalConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake
)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/CaptalConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${TO_EXPORT}
        EXPORT CaptalTargets
        ARCHIVE DESTINATION "lib/${CMAKE_BUILD_TYPE}"
        RUNTIME DESTINATION "bin/${CMAKE_BUILD_TYPE}"
        LIBRARY DESTINATION "bin/${CMAKE_BUILD_TYPE}"
        FRAMEWORK DESTINATION "lib/${CMAKE_BUILD_TYPE}"
        COMPONENT library)

install(EXPORT CaptalTargets
        DESTINATION lib/cmake/captal
        NAMESPACE Captal::)

install(FILES
        ${PROJECT_BINARY_DIR}/CaptalConfig.cmake
        ${PROJECT_BINARY_DIR}/CaptalConfigVersion.cmake
        DESTINATION lib/cmake/captal)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    if(CAPTAL_BUILD_APYRE)
        install(FILES $<TARGET_PDB_FILE:Apyre> DESTINATION bin/${CMAKE_BUILD_TYPE} OPTIONAL)
    endif()

    if(CAPTAL_BUILD_TEPHRA)
        install(FILES $<TARGET_PDB_FILE:Tephra> DESTINATION bin/${CMAKE_BUILD_TYPE} OPTIONAL)
    endif()

    if(CAPTAL_BUILD_SWELL)
        install(FILES $<TARGET_PDB_FILE:Swell> DESTINATION bin/${CMAKE_BUILD_TYPE} OPTIONAL)
    endif()

    if(CAPTAL_BUILD_CAPTAL)
        install(FILES $<TARGET_PDB_FILE:Captal> DESTINATION bin/${CMAKE_BUILD_TYPE} OPTIONAL)
    endif()
endif()

export(PACKAGE Captal)
