cmake_minimum_required(VERSION 3.21.0)

project(Captal VERSION 0.1.0)

find_package(ZLIB REQUIRED)

if(CAPTAL_INTERNAL_FREETYPE)
    find_package(Freetype REQUIRED NO_DEFAULT_PATH)
else()
    find_package(Freetype REQUIRED)
endif()

find_package(Chipmunk REQUIRED)
find_package(EnTT REQUIRED)
find_package(PalSigslot REQUIRED)
find_package(FastFloat REQUIRED)
find_package(pugixml REQUIRED)

set(CAPTAL_SOURCES
    #Headers:
    src/captal/config.hpp
    src/captal/algorithm.hpp
    src/captal/application.hpp
    src/captal/memory_transfer.hpp
    src/captal/buffer_pool.hpp
    src/captal/engine.hpp
    src/captal/zlib.hpp
    src/captal/translation.hpp
    src/captal/asynchronous_resource.hpp
    src/captal/push_constant_buffer.hpp
    src/captal/render_technique.hpp
    src/captal/render_target.hpp
    src/captal/render_window.hpp
    src/captal/render_texture.hpp
    src/captal/state.hpp
    src/captal/color.hpp
    src/captal/vertex.hpp
    src/captal/texture.hpp
    src/captal/window.hpp
    src/captal/uniform_buffer.hpp
    src/captal/storage_buffer.hpp
    src/captal/binding.hpp
    src/captal/shapes.hpp
    src/captal/renderable.hpp
    src/captal/view.hpp
    src/captal/bin_packing.hpp
    src/captal/font.hpp
    src/captal/text.hpp
    src/captal/sound.hpp
    src/captal/tiled_map.hpp
    src/captal/physics.hpp
    src/captal/widgets.hpp
    src/captal/signal.hpp

    src/captal/components/node.hpp
    src/captal/components/draw_index.hpp
    src/captal/components/listener.hpp
    src/captal/components/rigid_body.hpp
    src/captal/components/controller.hpp
    src/captal/components/attachment.hpp
    src/captal/components/drawable.hpp
    src/captal/components/camera.hpp
    src/captal/components/audio_emitter.hpp

    src/captal/systems/frame.hpp
    src/captal/systems/sorting.hpp
    src/captal/systems/audio.hpp
    src/captal/systems/render.hpp
    src/captal/systems/physics.hpp

    #Sources:
    src/captal/application.cpp
    src/captal/memory_transfer.cpp
    src/captal/buffer_pool.cpp
    src/captal/engine.cpp
    src/captal/zlib.cpp
    src/captal/translation.cpp
    src/captal/render_technique.cpp
    src/captal/render_target.cpp
    src/captal/render_window.cpp
    src/captal/render_texture.cpp
    src/captal/texture.cpp
    src/captal/window.cpp
    src/captal/uniform_buffer.cpp
    src/captal/storage_buffer.cpp
    src/captal/shapes.cpp
    src/captal/renderable.cpp
    src/captal/view.cpp
    src/captal/bin_packing.cpp
    src/captal/font.cpp
    src/captal/text.cpp
    src/captal/sound.cpp
    src/captal/tiled_map.cpp
    src/captal/physics.cpp
    src/captal/widgets.cpp
)

if(CAPTAL_BUILD_CAPTAL_STATIC)
    add_library(Captal STATIC ${CAPTAL_SOURCES})
    target_compile_definitions(Captal PUBLIC CAPTAL_STATIC_BUILD)
else()
    add_library(Captal SHARED ${CAPTAL_SOURCES})
    target_compile_definitions(Captal PUBLIC CAPTAL_SHARED_BUILD)
    set_target_properties(Captal PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set_target_properties(Captal PROPERTIES PREFIX "" OUTPUT_NAME "captal")

if(WIN32)
    target_sources(Captal PRIVATE src/captal/resources.rc)
    target_compile_definitions(Captal PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_compile_definitions(Captal
    PUBLIC
        $<$<CONFIG:Debug>:CAPTAL_DEBUG>
        CAPTAL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        CAPTAL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        CAPTAL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(Captal
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Captal
    PUBLIC
        Tephra
        Swell
        Apyre
        EnTT::EnTT
        Pal::Sigslot
    PRIVATE
        freetype
        chipmunk::chipmunk_static
        ZLIB::ZLIB
        pugixml::pugixml
        FastFloat::fast_float
        NotEnoughStandards::NotEnoughStandards
)

if(CAPTAL_BUILD_CAPTAL_EXAMPLES)
    add_library(CaptalSansation STATIC sansation.hpp sansation.cpp)

    add_executable(CaptalExample main.cpp)
    target_link_libraries(CaptalExample PRIVATE Captal CaptalSansation)
    target_include_directories(CaptalExample PRIVATE ${GLOBAL_INCLUDES})

    add_executable(CaptalExampleEasy example_easy.cpp)
    target_link_libraries(CaptalExampleEasy PRIVATE Captal CaptalSansation)
    target_include_directories(CaptalExampleEasy PRIVATE ${GLOBAL_INCLUDES})

    add_executable(CaptalText text.cpp)
    target_link_libraries(CaptalText PRIVATE Captal CaptalSansation)
    target_include_directories(CaptalText PRIVATE ${GLOBAL_INCLUDES})

    add_executable(CaptalWidgets widgets.cpp)
    target_link_libraries(CaptalWidgets PRIVATE Captal CaptalSansation)
    target_include_directories(CaptalWidgets PRIVATE ${GLOBAL_INCLUDES})
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/captal
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp)
