cmake_minimum_required(VERSION 3.0.0)

project(captal_engine)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(captal_foundation)
add_subdirectory(tephra)
add_subdirectory(swell)
add_subdirectory(apyre)
add_subdirectory(captal)
