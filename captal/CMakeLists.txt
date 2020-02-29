cmake_minimum_required(VERSION 3.0.0)

project(captal)

option(CAPTAL_BUILD_EXAMPLE OFF)


set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

set(FREETYPE_INCLUDES "${GLOBAL_INCLUDES}/freetype")
set(FREETYPE_LIBRARIES "${GLOBAL_LIBRARIES}/libfreetype.a")

set(CHIPMUNK_LIBRARIES "${GLOBAL_LIBRARIES}/libchipmunk.a")
set(ZLIB_LIBRARIES "${GLOBAL_LIBRARIES}/libzlibstatic.a")

set(HEADERS
    "src/config.hpp"
    "src/algorithm.hpp"
    "src/encoding.hpp"
    "src/application.hpp"
    "src/engine.hpp"
    "src/asynchronous_resource.hpp"
    "src/render_technique.hpp"
    "src/render_target.hpp"
    "src/render_window.hpp"
    "src/render_texture.hpp"
    "src/state.hpp"
    "src/color.hpp"
    "src/vertex.hpp"
    "src/texture.hpp"
    "src/framed_buffer.hpp"
    "src/uniform_buffer.hpp"
    "src/renderable.hpp"
    "src/view.hpp"
    "src/text.hpp"
    "src/sprite.hpp"
    "src/tileset.hpp"
    "src/tilemap.hpp"
    "src/sound.hpp"
    "src/tiled_map.hpp"
    "src/physics.hpp"

    "src/components/node.hpp"
    "src/components/drawable.hpp"
    "src/components/camera.hpp"
    "src/components/listener.hpp"
    "src/components/audio_emiter.hpp"
    "src/components/physical_body.hpp"

    "src/systems/frame.hpp"
    "src/systems/sorting.hpp"
    "src/systems/audio.hpp"
    "src/systems/render.hpp"
    "src/systems/physics.hpp"
    )

set(SOURCES
    "src/application.cpp"
    "src/engine.cpp"
    "src/render_technique.cpp"
    "src/render_target.cpp"
    "src/render_window.cpp"
    "src/render_texture.cpp"
    "src/state.cpp"
    "src/texture.cpp"
    "src/framed_buffer.cpp"
    "src/renderable.cpp"
    "src/view.cpp"
    "src/text.cpp"
    "src/sprite.cpp"
    "src/tileset.cpp"
    "src/tilemap.cpp"
    "src/sound.cpp"
    "src/tiled_map.cpp"
    "src/physics.cpp"

    "src/systems/frame.cpp"
    "src/systems/sorting.cpp"
    "src/systems/audio.cpp"
    "src/systems/render.cpp"
    "src/systems/physics.cpp"
    )

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS
                                                   GLM_FORCE_DEPTH_ZERO_TO_ONE
                                                   ZLIB_CONST
                                                   CAPTAL_SHARED_BUILD)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DCAPTAL_DEBUG)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${GLOBAL_INCLUDES} ${FREETYPE_INCLUDES})

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libapyre.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libswell.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/debug/libtephra.dll.a")
else()
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libapyre.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libswell.dll.a")
    target_link_libraries(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/../libs/release/libtephra.dll.a")
endif()

target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES} ${CHIPMUNK_LIBRARIES} ${ZLIB_LIBRARIES})

if(CAPTAL_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_test "main.cpp")
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/debug/libapyre.dll.a")
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/debug/libswell.dll.a")
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/debug/libtephra.dll.a")
    else()
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/release/libapyre.dll.a")
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/release/libswell.dll.a")
        target_link_libraries(${PROJECT_NAME}_test "${PROJECT_SOURCE_DIR}/../libs/release/libtephra.dll.a")
    endif()
endif()

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/debug"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        COMPONENT library)

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/release"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        COMPONENT library)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/../libs/include/${PROJECT_NAME}"
        FILES_MATCHING PATTERN "*.hpp")
