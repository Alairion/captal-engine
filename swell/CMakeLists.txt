cmake_minimum_required(VERSION 3.15.7)

project(swell VERSION 0.1.0)

include(cmake/buildportaudio.cmake)
include(cmake/buildogg.cmake)
include(cmake/buildvorbis.cmake)
include(cmake/buildflac.cmake)

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
    add_library(swell STATIC ${SWELL_SOURCES})
    target_compile_definitions(swell PUBLIC SWELL_STATIC_BUILD)
    set(TO_EXPORT ${TO_EXPORT} swell portaudio_static vorbisfile vorbis ogg not_enough_standards PARENT_SCOPE)
else()
    add_library(swell SHARED ${SWELL_SOURCES})
    target_compile_definitions(swell PUBLIC SWELL_SHARED_BUILD)
    set(TO_EXPORT ${TO_EXPORT} swell not_enough_standards PARENT_SCOPE)
endif()

set_target_properties(swell PROPERTIES PREFIX "")

target_compile_definitions(swell
    PRIVATE
        OV_EXCLUDE_STATIC_CALLBACKS
    PUBLIC
        SWELL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        SWELL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        SWELL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(swell
    SYSTEM PRIVATE
        ${PORTAUDIO_INCLUDES}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(swell
    PRIVATE
        portaudio_static
        vorbisfile
        FLAC
    PUBLIC
        captal_foundation
        not_enough_standards
)

if(WIN32)
    target_sources(swell PRIVATE src/swell/resources.rc)
    target_compile_definitions(swell PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(CAPTAL_BUILD_SWELL_EXAMPLES)
    add_executable(swell_test "main.cpp")
    target_link_libraries(swell_test swell)
endif()

install(TARGETS swell
        CONFIGURATIONS Debug
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        COMPONENT library)

install(TARGETS swell
        CONFIGURATIONS Release
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        COMPONENT library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/swell
        DESTINATION ${PROJECT_SOURCE_DIR}/include
        FILES_MATCHING PATTERN *.hpp)
