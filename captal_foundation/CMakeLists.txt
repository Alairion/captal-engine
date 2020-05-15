cmake_minimum_required(VERSION 3.0.0)

project(captal_foundation)

option(CAPTAL_FOUNDATION_BUILD_EXAMPLES ON)

set(CAPTAL_FOUNDATION_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src")
set(HEADERS "${CAPTAL_FOUNDATION_INCLUDE_DIR}/config.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/optional_ref.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/enum_operations.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/version.hpp"
            "${CAPTAL_FOUNDATION_INCLUDE_DIR}/endian.hpp")

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${HEADERS})

if(CAPTAL_FOUNDATION_BUILD_EXAMPLES)
    add_executable(${PROJECT_NAME}_test "main.cpp")
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
endif()

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/../libs/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")
