cmake_minimum_required(VERSION 3.21.0)

project(Swell VERSION 0.1.0)

find_package(NotEnoughStandards REQUIRED)
find_package(portaudio REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)
find_package(FLAC REQUIRED)

set(SWELL_SOURCES
    #Headers
    src/swell/config.hpp
    src/swell/application.hpp
    src/swell/physical_device.hpp
    src/swell/audio_world.hpp
    src/swell/sound_reader.hpp
    src/swell/stream.hpp
    src/swell/audio_pulser.hpp
    src/swell/wave.hpp
    src/swell/ogg.hpp
    src/swell/flac.hpp
    src/swell/sound_file.hpp

    #Sources:
    src/swell/application.cpp
    src/swell/audio_world.cpp
    src/swell/stream.cpp
    src/swell/audio_pulser.cpp
    src/swell/wave.cpp
    src/swell/ogg.cpp
    src/swell/flac.cpp
    src/swell/sound_file.cpp
)

if(CAPTAL_BUILD_SWELL_STATIC)
    add_library(Swell STATIC ${SWELL_SOURCES})
    target_compile_definitions(Swell PUBLIC SWELL_STATIC_BUILD)
else()
    add_library(Swell SHARED ${SWELL_SOURCES})
    target_compile_definitions(Swell PUBLIC SWELL_SHARED_BUILD)
    set_target_properties(Swell PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set_target_properties(Swell PROPERTIES PREFIX "" OUTPUT_NAME "swell")

target_compile_definitions(Swell
    PUBLIC
        SWELL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        SWELL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        SWELL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(Swell
    SYSTEM PRIVATE
        ${PORTAUDIO_INCLUDES}
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Swell
    PRIVATE
        portaudio_static
        Vorbis::vorbisfile
        FLAC::FLAC
    PUBLIC
        CaptalFoundation
        NotEnoughStandards::NotEnoughStandards
)

if(WIN32)
    target_sources(Swell PRIVATE src/swell/resources.rc)
    target_compile_definitions(Swell PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(CAPTAL_BUILD_SWELL_EXAMPLES)
    add_executable(SwellTest "main.cpp")
    target_link_libraries(SwellTest Swell)
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/swell
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp)
