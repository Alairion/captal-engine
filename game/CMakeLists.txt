cmake_minimum_required(VERSION 3.0)

project(my_project VERSION 0.0.1)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

set(HEADERS
    "src/config.hpp"
    "src/viewport_compute.hpp"

    "src/map.hpp"
    "src/clock.hpp"

    "src/states/splash_screen.hpp"
    "src/states/main_menu.hpp"
    "src/states/game.hpp"

    "src/components/player_controller.hpp"
    )

set(SOURCES
    "src/main.cpp"

    "src/map.cpp"
    "src/clock.cpp"

    "src/states/splash_screen.cpp"
    "src/states/main_menu.cpp"
    "src/states/game.cpp"
    )

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

add_compile_definitions(MY_PROJECT_MAJOR_VERSION=${PROJECT_VERSION_MAJOR})
add_compile_definitions(MY_PROJECT_MINOR_VERSION=${PROJECT_VERSION_MINOR})
add_compile_definitions(MY_PROJECT_PATCH_VERSION=${PROJECT_VERSION_PATCH})
add_compile_definitions(FMT_HEADER_ONLY)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLOBAL_INCLUDES})

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libapyre.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libswell.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libtephra.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libcaptal.dll.a")
else()
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libapyre.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libswell.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libtephra.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libcaptal.dll.a")
endif()

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/debug"
        COMPONENT executable)

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/release"
        COMPONENT executable)
