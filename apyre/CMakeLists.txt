cmake_minimum_required(VERSION 3.0.0)

project(apyre VERSION 0.1.0)

option(APYRE_BUILD_EXAMPLE ON)

set(3DNOW                  OFF CACHE INTERNAL "")
set(ALSA                   OFF CACHE INTERNAL "")
set(ALSA_SHARED            OFF CACHE INTERNAL "")
set(ALTIVEC                ON  CACHE INTERNAL "")
set(ARMNEON                OFF CACHE INTERNAL "")
set(ARMSIMD                OFF CACHE INTERNAL "")
set(ARTS                   OFF CACHE INTERNAL "")
set(ARTS_SHARED            OFF CACHE INTERNAL "")
set(ASSEMBLY               ON  CACHE INTERNAL "")
set(ASSERTIONS             "disabled" CACHE INTERNAL "")
set(BACKGROUNDING_SIGNAL   OFF CACHE INTERNAL "")
set(CLOCK_GETTIME          OFF CACHE INTERNAL "")
set(DIRECTFB_SHARED        OFF CACHE INTERNAL "")
set(DIRECTX                OFF CACHE INTERNAL "")
set(DISKAUDIO              OFF CACHE INTERNAL "")
set(DUMMYAUDIO             OFF CACHE INTERNAL "")
set(ESD                    OFF CACHE INTERNAL "")
set(ESD_SHARED             OFF CACHE INTERNAL "")
set(FOREGROUNDING_SIGNAL   OFF CACHE INTERNAL "")
set(FUSIONSOUND            OFF CACHE INTERNAL "")
set(FUSIONSOUND_SHARED     OFF CACHE INTERNAL "")
set(GCC_ATOMICS            ON  CACHE INTERNAL "")
set(HIDAPI                 ON  CACHE INTERNAL "")
set(INPUT_TSLIB            OFF CACHE INTERNAL "")
set(JACK                   OFF CACHE INTERNAL "")
set(JACK_SHARED            OFF CACHE INTERNAL "")
set(JOYSTICK_VIRTUAL       ON  CACHE INTERNAL "")
set(KMSDRM_SHARED          OFF CACHE INTERNAL "")
set(LIBC                   ON  CACHE INTERNAL "")
set(LIBSAMPLERATE          OFF CACHE INTERNAL "")
set(LIBSAMPLERATE_SHARED   OFF CACHE INTERNAL "")
set(MMX                    ON  CACHE INTERNAL "")
set(NAS                    OFF CACHE INTERNAL "")
set(NAS_SHARED             OFF CACHE INTERNAL "")
set(OSS                    OFF CACHE INTERNAL "")
set(PTHREADS               OFF CACHE INTERNAL "")
set(PTHREADS_SEM           OFF CACHE INTERNAL "")
set(PULSEAUDIO             OFF CACHE INTERNAL "")
set(PULSEAUDIO_SHARED      OFF CACHE INTERNAL "")
set(RENDER_D3D             OFF CACHE INTERNAL "")
set(RENDER_METAL           OFF CACHE INTERNAL "")
set(RPATH                  OFF CACHE INTERNAL "")
set(SDL_ATOMIC             ON  CACHE INTERNAL "")
set(SDL_AUDIO              OFF CACHE INTERNAL "")
set(SDL_CMAKE_DEBUG_POSTFIX "d" CACHE INTERNAL "")
set(SDL_CPUINFO            ON  CACHE INTERNAL "")
set(SDL_DLOPEN             ON  CACHE INTERNAL "")
set(SDL_EVENTS             ON  CACHE INTERNAL "")
set(SDL_FILE               ON  CACHE INTERNAL "")
set(SDL_FILESYSTEM         OFF CACHE INTERNAL "")
set(SDL_HAPTIC             ON  CACHE INTERNAL "")
set(SDL_JOYSTICK           ON  CACHE INTERNAL "")
set(SDL_LOADSO             ON  CACHE INTERNAL "")
set(SDL_LOCALE             ON  CACHE INTERNAL "")
set(SDL_POWER              ON  CACHE INTERNAL "")
set(SDL_RENDER             OFF CACHE INTERNAL "")
set(SDL_SENSOR             ON  CACHE INTERNAL "")
set(SDL_SHARED             OFF CACHE INTERNAL "")
set(SDL_STATIC             ON  CACHE INTERNAL "")
set(SDL_STATIC_PIC         OFF CACHE INTERNAL "")
set(SDL_TEST               OFF CACHE INTERNAL "")
set(SDL_THREADS            ON  CACHE INTERNAL "")
set(SDL_TIMERS             ON  CACHE INTERNAL "")
set(SDL_VIDEO              ON  CACHE INTERNAL "")
set(SNDIO                  OFF CACHE INTERNAL "")
set(SNDIO_SHARED           OFF CACHE INTERNAL "")
set(SSE                    ON  CACHE INTERNAL "")
set(SSE2                   ON  CACHE INTERNAL "")
set(SSE3                   ON  CACHE INTERNAL "")
set(SSEMATH                ON  CACHE INTERNAL "")
set(VIDEO_COCOA            OFF CACHE INTERNAL "")
set(VIDEO_DIRECTFB         OFF CACHE INTERNAL "")
set(VIDEO_DUMMY            OFF CACHE INTERNAL "")
set(VIDEO_KMSDRM           OFF CACHE INTERNAL "")
set(VIDEO_METAL            OFF CACHE INTERNAL "")
set(VIDEO_OFFSCREEN        OFF CACHE INTERNAL "")
set(VIDEO_OPENGL           OFF CACHE INTERNAL "")
set(VIDEO_OPENGLES         OFF CACHE INTERNAL "")
set(VIDEO_RPI              OFF CACHE INTERNAL "")
set(VIDEO_VIVANTE          OFF CACHE INTERNAL "")
set(VIDEO_VULKAN           ON  CACHE INTERNAL "")
set(VIDEO_WAYLAND          OFF CACHE INTERNAL "")
set(VIDEO_WAYLAND_QT_TOUCH OFF CACHE INTERNAL "")
set(VIDEO_X11              OFF CACHE INTERNAL "")
set(VIDEO_X11_XCURSOR      OFF CACHE INTERNAL "")
set(VIDEO_X11_XINERAMA     OFF CACHE INTERNAL "")
set(VIDEO_X11_XINPUT       OFF CACHE INTERNAL "")
set(VIDEO_X11_XRANDR       OFF CACHE INTERNAL "")
set(VIDEO_X11_XSCRNSAVER   OFF CACHE INTERNAL "")
set(VIDEO_X11_XSHAPE       OFF CACHE INTERNAL "")
set(VIDEO_X11_XVM          OFF CACHE INTERNAL "")
set(WASAPI                 OFF CACHE INTERNAL "")
set(WAYLAND_SHARED         OFF CACHE INTERNAL "")
set(X11_SHARED             OFF CACHE INTERNAL "")

add_subdirectory(external/sdl EXCLUDE_FROM_ALL)

add_library(apyre
    #Headers:
    src/apyre/config.hpp
    src/apyre/application.hpp
    src/apyre/monitor.hpp
    src/apyre/window.hpp
    src/apyre/event.hpp
    src/apyre/messagebox.hpp

    #Sources:
    src/apyre/application.cpp
    src/apyre/window.cpp
    src/apyre/event.cpp
    src/apyre/messagebox.cpp
)

set_target_properties(apyre PROPERTIES PREFIX "")

target_compile_definitions(apyre
    PRIVATE
        CAPTAL_FOUNDATION_SHARED_BUILD
    PUBLIC
        APYRE_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        APYRE_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        APYRE_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(apyre
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(apyre
    PRIVATE
        SDL2main
        SDL2-static
    PUBLIC
        captal_foundation
)

if(WIN32)
    target_sources(apyre PRIVATE src/apyre/resources.rc)
endif()

if(APYRE_BUILD_EXAMPLE)
    add_executable(apyre_test "main.cpp")
    target_link_libraries(apyre_test PRIVATE apyre)
    target_include_directories(apyre_test PRIVATE ${GLOBAL_INCLUDES})
endif()

install(TARGETS apyre
        CONFIGURATIONS Debug
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        COMPONENT library)

install(TARGETS apyre
        CONFIGURATIONS Release
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        COMPONENT library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/apyre
        DESTINATION ${PROJECT_SOURCE_DIR}/include
        FILES_MATCHING PATTERN *.hpp)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/apyre
        DESTINATION ${PROJECT_SOURCE_DIR}/../libs/include
        FILES_MATCHING PATTERN *.hpp)
