cmake_minimum_required(VERSION 3.13)

project(captal_engine VERSION 0.1.0
                      DESCRIPTION "Captal Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/buildnes.cmake)
include(cmake/download_submodule.cmake)

add_subdirectory(captal_foundation)
add_subdirectory(tephra)
add_subdirectory(swell)
add_subdirectory(apyre)
add_subdirectory(captal)
