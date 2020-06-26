cmake_minimum_required(VERSION 3.0.0)

project(captal VERSION 0.1.0)

option(CAPTAL_BUILD_EXAMPLE OFF)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

set(FREETYPE_INCLUDES "${GLOBAL_INCLUDES}/freetype")
set(FREETYPE_LIBRARIES "${GLOBAL_LIBRARIES}/libfreetype.a")

set(CHIPMUNK_LIBRARIES "${GLOBAL_LIBRARIES}/libchipmunk.a")
set(ZLIB_LIBRARIES "${GLOBAL_LIBRARIES}/libzlibstatic.a")

add_library(captal SHARED
    #Headers:
    src/captal/config.hpp
    src/captal/algorithm.hpp
    src/captal/application.hpp
    src/captal/engine.hpp
    src/captal/zlib.hpp
    src/captal/translation.hpp
    src/captal/asynchronous_resource.hpp
    src/captal/render_technique.hpp
    src/captal/render_target.hpp
    src/captal/render_window.hpp
    src/captal/render_texture.hpp
    src/captal/state.hpp
    src/captal/color.hpp
    src/captal/vertex.hpp
    src/captal/texture.hpp
    src/captal/framed_buffer.hpp
    src/captal/uniform_buffer.hpp
    src/captal/shapes.hpp
    src/captal/renderable.hpp
    src/captal/view.hpp
    src/captal/text.hpp
    src/captal/sound.hpp
    src/captal/tiled_map.hpp
    src/captal/physics.hpp

    src/captal/components/node.hpp
    src/captal/components/drawable.hpp
    src/captal/components/draw_index.hpp
    src/captal/components/camera.hpp
    src/captal/components/listener.hpp
    src/captal/components/audio_emiter.hpp
    src/captal/components/physical_body.hpp
    src/captal/components/controller.hpp

    src/captal/systems/frame.hpp
    src/captal/systems/sorting.hpp
    src/captal/systems/audio.hpp
    src/captal/systems/render.hpp
    src/captal/systems/physics.hpp

    #Sources:
    src/captal/application.cpp
    src/captal/engine.cpp
    src/captal/zlib.cpp
    src/captal/translation.cpp
    src/captal/render_technique.cpp
    src/captal/render_target.cpp
    src/captal/render_window.cpp
    src/captal/render_texture.cpp
    src/captal/state.cpp
    src/captal/texture.cpp
    src/captal/framed_buffer.cpp
    src/captal/shapes.cpp
    src/captal/renderable.cpp
    src/captal/view.cpp
    src/captal/text.cpp
    src/captal/sound.cpp
    src/captal/tiled_map.cpp
    src/captal/physics.cpp
)

if(WIN32)
    target_sources(captal PRIVATE src/captal/resources.rc)
endif()

target_compile_definitions(captal
    PRIVATE
        CAPTAL_FOUNDATION_SHARED_BUILD
        ZLIB_CONST
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
    PUBLIC
        CAPTAL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        CAPTAL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        CAPTAL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

set_target_properties(captal PROPERTIES PREFIX "")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(captal PRIVATE CAPTAL_DEBUG)
endif()

target_include_directories(captal
    SYSTEM PRIVATE
        ${GLOBAL_INCLUDES} ${FREETYPE_INCLUDES}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(captal
    PUBLIC
        captal_foundation
        tephra
        swell
        apyre
    PRIVATE
        ${FREETYPE_LIBRARIES}
        ${CHIPMUNK_LIBRARIES}
        ${ZLIB_LIBRARIES}
)

install(TARGETS captal
        CONFIGURATIONS Debug
        RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../libs/debug
        ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/../libs/debug
        COMPONENT library)

install(TARGETS captal
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../libs/release
        ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/../libs/release
        COMPONENT library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/captal
        DESTINATION ${PROJECT_SOURCE_DIR}/include
        FILES_MATCHING PATTERN *.hpp)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/captal
        DESTINATION ${PROJECT_SOURCE_DIR}/../libs/include
        FILES_MATCHING PATTERN *.hpp)

if(CAPTAL_BUILD_EXAMPLE)
    add_executable(captal_test "main.cpp")
    target_link_libraries(captal_test PRIVATE captal)
endif()
