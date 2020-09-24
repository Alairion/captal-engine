cmake_minimum_required(VERSION 3.14)

project(swell VERSION 0.1.0)

include(cmake/buildportaudio.cmake)
include(cmake/buildogg.cmake)
include(cmake/buildvorbis.cmake)

set(PORTAUDIO_INCLUDES "${GLOBAL_INCLUDES}/portaudio")
set(PORTAUDIO_LIBRARIES "${GLOBAL_LIBRARIES}/libportaudio_static.a" ole32 uuid setupapi winmm)
set(VORBISENC_LIBRARIES "${GLOBAL_LIBRARIES}/libvorbisenc.a")
set(VORBISFILE_LIBRARIES "${GLOBAL_LIBRARIES}/libvorbisfile.a")
set(VORBIS_LIBRARIES "${GLOBAL_LIBRARIES}/libvorbis.a")
set(OGG_LIBRARIES "${GLOBAL_LIBRARIES}/libogg.a")

#[[
set(BUILD_CXXLIBS OFF CACHE BOOL "Flac options")
set(BUILD_DOCS OFF CACHE BOOL "Flac options")
set(BUILD_EXAMPLES OFF CACHE BOOL "Flac options")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Flac options")
set(BUILD_TESTING OFF CACHE BOOL "Flac options")
set(WITH_ASM ON CACHE BOOL "Flac options")
set(WITH_AVX ON CACHE BOOL "Flac options")
set(WITH_OGG OFF CACHE BOOL "Flac options")
set(WITH_STACK_PROTECTOR OFF CACHE BOOL "Flac options")
set(WITH_XMMS OFF CACHE BOOL "Flac options")
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/flac" EXCLUDE_FROM_ALL)
]]

option(SWELL_BUILD_EXAMPLE OFF)

add_library(swell SHARED
    #Headers
    src/swell/config.hpp
    src/swell/application.hpp
    src/swell/physical_device.hpp
    src/swell/sound_reader.hpp
    src/swell/mixer.hpp
    src/swell/stream.hpp
    src/swell/wave.hpp
    src/swell/ogg.hpp
    src/swell/sound_file.hpp

    #Sources:
    src/swell/application.cpp
    src/swell/mixer.cpp
    src/swell/stream.cpp
    src/swell/wave.cpp
    src/swell/ogg.cpp
    src/swell/sound_file.cpp
)

set_target_properties(swell PROPERTIES PREFIX "")

target_compile_definitions(swell
    PRIVATE
        CAPTAL_FOUNDATION_SHARED_BUILD
    PUBLIC
        SWELL_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        SWELL_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        SWELL_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(swell
    SYSTEM PRIVATE
        ${PORTAUDIO_INCLUDES}
        ${GLOBAL_INCLUDES}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(swell
    PRIVATE
        ${PORTAUDIO_LIBRARIES} ${VORBISENC_LIBRARIES} ${VORBISFILE_LIBRARIES} ${VORBIS_LIBRARIES} ${OGG_LIBRARIES}
    PUBLIC
        captal_foundation
)

if(WIN32)
    target_sources(swell PRIVATE src/swell/resources.rc)
endif()

if(SWELL_BUILD_EXAMPLE)
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
