cmake_minimum_required(VERSION 3.0.0)

project(tephra VERSION 0.1.0)

option(TEPHRA_BUILD_EXAMPLE OFF)
option(TEPHRA_USE_PLATFORM_ANDROID "If on, enable the Android surface extension" OFF)
option(TEPHRA_USE_PLATFORM_IOS "If on, enable the IOS surface extension" OFF)
option(TEPHRA_USE_PLATFORM_WIN32 "If on, enable the Windows surface extension" OFF)
option(TEPHRA_USE_PLATFORM_MACOS "If on, enable the OSX surface extension" OFF)
option(TEPHRA_USE_PLATFORM_XLIB "If on, enable the Linux Xlib surface extension" OFF)
option(TEPHRA_USE_PLATFORM_XCB "If on, enable the Linux XCB surface extension" OFF)
option(TEPHRA_USE_PLATFORM_WAYLAND "If on, enable the Linux Wayland surface extension" OFF)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

if(TEPHRA_BUILD_EXAMPLE)
    set(SDL_INCLUDES "${GLOBAL_INCLUDES}/sdl")
    set(SDL_LIBRARIES "${GLOBAL_LIBRARIES}/libSDL2main.a" "${GLOBAL_LIBRARIES}/libSDL2.a" "${GLOBAL_LIBRARIES}/libSDL2-static.a")
endif()

add_library(tephra SHARED
    #Headers:
    src/tephra/config.hpp
    src/tephra/enumerations.hpp
    src/tephra/hardware.hpp
    src/tephra/surface.hpp
    src/tephra/swapchain.hpp
    src/tephra/application.hpp
    src/tephra/renderer.hpp
    src/tephra/shader.hpp
    src/tephra/render_target.hpp
    src/tephra/synchronization.hpp
    src/tephra/descriptor.hpp
    src/tephra/pipeline.hpp
    src/tephra/buffer.hpp
    src/tephra/image.hpp
    src/tephra/texture.hpp
    src/tephra/commands.hpp

    src/tephra/vulkan/vulkan.hpp
    src/tephra/vulkan/vulkan_functions.hpp
    src/tephra/vulkan/memory.hpp
    src/tephra/vulkan/helper.hpp

    #Sources:
    src/tephra/hardware.cpp
    src/tephra/surface.cpp
    src/tephra/swapchain.cpp
    src/tephra/application.cpp
    src/tephra/renderer.cpp
    src/tephra/shader.cpp
    src/tephra/render_target.cpp
    src/tephra/synchronization.cpp
    src/tephra/descriptor.cpp
    src/tephra/pipeline.cpp
    src/tephra/buffer.cpp
    src/tephra/image.cpp
    src/tephra/texture.cpp
    src/tephra/commands.cpp

    src/tephra/vulkan/vulkan.cpp
    src/tephra/vulkan/vulkan_functions.cpp
    src/tephra/vulkan/memory.cpp
    src/tephra/vulkan/helper.cpp
)

set_target_properties(tephra PROPERTIES PREFIX "")

target_compile_definitions(tephra
    PRIVATE
        CAPTAL_FOUNDATION_SHARED_BUILD
    PUBLIC
        VK_NO_PROTOTYPES
        TEPHRA_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        TEPHRA_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        TEPHRA_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_link_libraries(tephra PUBLIC captal_foundation)

if(TEPHRA_USE_PLATFORM_ANDROID)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_ANDROID
        PRIVATE
            VK_USE_PLATFORM_ANDROID_KHR
    )
endif()

if(TEPHRA_USE_PLATFORM_IOS)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_IOS
        PRIVATE
            VK_USE_PLATFORM_IOS_MVK
    )
endif()

if(TEPHRA_USE_PLATFORM_WIN32)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_WIN32
        PRIVATE
            VK_USE_PLATFORM_WIN32_KHR
    )
endif()

if(TEPHRA_USE_PLATFORM_MACOS)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_MACOS
        PRIVATE
            VK_USE_PLATFORM_MACOS_MVK
    )
endif()

if(TEPHRA_USE_PLATFORM_XLIB)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_XLIB
        PRIVATE
            VK_USE_PLATFORM_XLIB_KHR
    )
endif()

if(TEPHRA_USE_PLATFORM_XCB)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_XCB
        PRIVATE
            VK_USE_PLATFORM_XCB_KHR
    )
endif()

if(TEPHRA_USE_PLATFORM_WAYLAND)
    target_compile_definitions(tephra
        PUBLIC
            TPH_PLATFORM_WAYLAND
        PRIVATE
            VK_USE_PLATFORM_WAYLAND_KHR
    )
endif()

target_include_directories(tephra
    SYSTEM PUBLIC
        ${GLOBAL_INCLUDES}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

if(WIN32)
    target_sources(tephra PRIVATE src/tephra/resources.rc)
endif()

if(TEPHRA_BUILD_EXAMPLE)
    add_executable(tephra_test "main.cpp" "window.hpp" "vertex.hpp")
    target_compile_definitions(tephra_test PRIVATE SDL_MAIN_HANDLED)
    target_include_directories(tephra_test PRIVATE ${SDL_INCLUDES} ${GLOBAL_INCLUDES})
    target_link_libraries(tephra_test tephra PRIVATE ${SDL_LIBRARIES})
endif()

install(TARGETS tephra
        CONFIGURATIONS Debug
        RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../libs/debug
        ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/../libs/debug
        COMPONENT library)

install(TARGETS tephra
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../libs/release
        ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/../libs/release
        COMPONENT library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src DESTINATION ${PROJECT_SOURCE_DIR}/include
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src DESTINATION ${PROJECT_SOURCE_DIR}/../libs/include
        FILES_MATCHING PATTERN "*.hpp")
