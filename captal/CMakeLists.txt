cmake_minimum_required(VERSION 3.15.7)

project(captal VERSION 0.1.0)

include(cmake/buildchipmunk.cmake)
include(cmake/buildfreetype.cmake)
include(cmake/buildzlib.cmake)
captal_download_submodule(captal/external/entt TRUE)
captal_download_submodule(captal/external/sigslots TRUE)

set(CAPTAL_SOURCES
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
    src/captal/uniform_buffer.hpp
    src/captal/storage_buffer.hpp
    src/captal/binding.hpp
    src/captal/shapes.hpp
    src/captal/renderable.hpp
    src/captal/view.hpp
    src/captal/text.hpp
    src/captal/sound.hpp
    src/captal/tiled_map.hpp
    src/captal/physics.hpp
    src/captal/widgets.hpp

    src/captal/components/node.hpp
    src/captal/components/draw_index.hpp
    src/captal/components/listener.hpp
    src/captal/components/rigid_body.hpp
    src/captal/components/controller.hpp
    src/captal/components/attachment.hpp
    src/captal/components/drawable.hpp
    src/captal/components/camera.hpp
    src/captal/components/audio_emiter.hpp

    src/captal/systems/frame.hpp
    src/captal/systems/sorting.hpp
    src/captal/systems/audio.hpp
    src/captal/systems/render.hpp
    src/captal/systems/physics.hpp

    src/captal/signal.hpp
    src/captal/entity.hpp

    #Sources:
    src/captal/application.cpp
    src/captal/engine.cpp
    src/captal/zlib.cpp
    src/captal/translation.cpp
    src/captal/render_technique.cpp
    src/captal/render_target.cpp
    src/captal/render_window.cpp
    src/captal/render_texture.cpp
    src/captal/texture.cpp
    src/captal/uniform_buffer.cpp
    src/captal/storage_buffer.cpp
    src/captal/shapes.cpp
    src/captal/renderable.cpp
    src/captal/view.cpp
    src/captal/text.cpp
    src/captal/sound.cpp
    src/captal/tiled_map.cpp
    src/captal/physics.cpp
    src/captal/widgets.cpp
)

if(CAPTAL_BUILD_CAPTAL_STATIC)
    add_library(captal STATIC ${CAPTAL_SOURCES})
else()
    add_library(captal SHARED ${CAPTAL_SOURCES})
    target_compile_definitions(captal PUBLIC CAPTAL_FOUNDATION_SHARED_BUILD)
endif()

set_target_properties(captal PROPERTIES PREFIX "")

if(WIN32)
    target_sources(captal PRIVATE src/captal/resources.rc)
endif()

target_compile_definitions(captal
    PUBLIC
        $<$<CONFIG:Debug>:CAPTAL_DEBUG>
        CAPTAL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        CAPTAL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        CAPTAL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(captal
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PUBLIC
        ${PROJECT_SOURCE_DIR}/external/entt/src
        ${PROJECT_SOURCE_DIR}/external/sigslots/include
    SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/external/chipmunk/include
)

target_link_libraries(captal
    PUBLIC
        tephra
        swell
        apyre
    PRIVATE
        freetype
        chipmunk_static
        zlib
        not_enough_standards
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

if(CAPTAL_BUILD_CAPTAL_EXAMPLES)
    add_executable(captal_test "main.cpp")
    target_link_libraries(captal_test PRIVATE captal)
    target_include_directories(captal_test PRIVATE ${GLOBAL_INCLUDES})
endif()
