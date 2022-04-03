cmake_minimum_required(VERSION 3.15.7)

project(captal_foundation VERSION 0.1.0)

add_library(captal_foundation INTERFACE)

target_include_directories(captal_foundation INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_sources(captal_foundation INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/base.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/encoding.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/enum_operations.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/optional_ref.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/stack_allocator.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/utility.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/math.hpp"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>$<INSTALL_INTERFACE:include>/captal_foundation/version.hpp"
)

if(CAPTAL_BUILD_FOUNDATION_TESTS)
    add_executable(captal_foundation_test ${PROJECT_SOURCE_DIR}/test.cpp)
    target_link_libraries(captal_foundation_test captal_foundation Catch2)
endif()

install(TARGETS captal_foundation
        INCLUDES DESTINATION ${PROJECT_SOURCE_DIR}/src)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/captal_foundation
        DESTINATION ${PROJECT_SOURCE_DIR}/src
        FILES_MATCHING PATTERN *.hpp)


