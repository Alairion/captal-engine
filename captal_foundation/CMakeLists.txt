cmake_minimum_required(VERSION 3.0.0)

project(captal_foundation)

set(CAPTAL_FOUNDATION_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src")
set(HEADERS "${CAPTAL_FOUNDATION_INCLUDE_DIR}/base.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/optional_ref.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/enum_operations.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/version.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/stack_allocator.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/encoding.hpp")

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${HEADERS})

add_executable(${PROJECT_NAME}_test "test.cpp")
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/../libs/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")
