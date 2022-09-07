cmake_minimum_required(VERSION 3.21.0)

project(CaptalFoundation VERSION 0.1.0)

add_library(CaptalFoundation INTERFACE)

target_include_directories(CaptalFoundation INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/captal_foundation>
)

target_sources(CaptalFoundation INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/base.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/encoding.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/enum_operations.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/optional_ref.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/stack_allocator.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/utility.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/math.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/captal_foundation/version.hpp>
)

if(CAPTAL_BUILD_FOUNDATION_TESTS)
    add_executable(CaptalFoundationTest ${PROJECT_SOURCE_DIR}/test.cpp)
    target_link_libraries(CaptalFoundationTest captal_foundation Catch2)
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/captal_foundation
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp)
