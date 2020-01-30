cmake_minimum_required(VERSION 3.14)

project(swell)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

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

set(HEADERS
    "src/config.hpp"
    "src/application.hpp"
    "src/physical_device.hpp"
    "src/mixer.hpp"
    "src/stream.hpp"
    "src/wave.hpp"
    "src/ogg.hpp"
    "src/sound_file.hpp")

set(SOURCES
    "src/application.cpp"
    "src/mixer.cpp"
    "src/stream.cpp"
    "src/wave.cpp"
    "src/ogg.cpp"
    "src/sound_file.cpp")

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PRIVATE ${PORTAUDIO_INCLUDES} ${GLOBAL_INCLUDES})

target_link_libraries(${PROJECT_NAME} ${PORTAUDIO_LIBRARIES} ${VORBISENC_LIBRARIES} ${VORBISFILE_LIBRARIES} ${VORBIS_LIBRARIES} ${OGG_LIBRARIES})

if(SWELL_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_test "main.cpp")
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
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
