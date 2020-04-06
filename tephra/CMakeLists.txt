cmake_minimum_required(VERSION 3.0.0)

project(tephra)
add_definitions(-DVK_NO_PROTOTYPES)

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

set(SDL_INCLUDES "${GLOBAL_INCLUDES}/sdl")
set(SDL_LIBRARIES "${GLOBAL_LIBRARIES}/libSDL2main.a"
                  "${GLOBAL_LIBRARIES}/libSDL2.a"
                  "${GLOBAL_LIBRARIES}/libSDL2-static.a")

if(MINGW)
    set(SDL_LIBRARIES ${SDL_LIBRARIES} imagehlp dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid)
endif()

set(HEADERS
    "src/vulkan/vulkan.hpp"
    "src/vulkan/vulkan_functions.hpp"
    "src/vulkan/memory.hpp"
    "src/vulkan/helper.hpp"
    "src/vulkan/render_target.hpp"

    "src/config.hpp"
    "src/enumerations.hpp"
    "src/hardware.hpp"
    "src/surface.hpp"
    "src/application.hpp"
    "src/renderer.hpp"
    "src/shader.hpp"
    "src/render_target.hpp"
    "src/synchronization.hpp"
    "src/descriptor.hpp"
    "src/pipeline.hpp"
    "src/buffer.hpp"
    "src/image.hpp"
    "src/texture.hpp"
    "src/commands.hpp"
    )

set(SOURCES
    "src/vulkan/vulkan.cpp"
    "src/vulkan/vulkan_functions.cpp"
    "src/vulkan/memory.cpp"
    "src/vulkan/helper.cpp"
    "src/vulkan/render_target.cpp"

    "src/hardware.cpp"
    "src/surface.cpp"
    "src/application.cpp"
    "src/renderer.cpp"
    "src/shader.cpp"
    "src/render_target.cpp"
    "src/synchronization.cpp"
    "src/descriptor.cpp"
    "src/pipeline.cpp"
    "src/buffer.cpp"
    "src/image.cpp"
    "src/texture.cpp"
    "src/commands.cpp"
    )

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(TEPHRA_USE_PLATFORM_ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_ANDROID_KHR)
endif()

if(TEPHRA_USE_PLATFORM_IOS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_IOS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_IOS_MVK)
endif()

if(TEPHRA_USE_PLATFORM_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
endif()

if(TEPHRA_USE_PLATFORM_MACOS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_MACOS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_MACOS_MVK)
endif()

if(TEPHRA_USE_PLATFORM_XLIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_XLIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

if(TEPHRA_USE_PLATFORM_XCB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_XCB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()

if(TEPHRA_USE_PLATFORM_WAYLAND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TPH_PLATFORM_WAYLAND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${GLOBAL_INCLUDES})

if(TEPHRA_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_test "main.cpp" "window.hpp" "vertex.hpp")
    add_definitions(-DSDL_MAIN_HANDLED)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${SDL_INCLUDES} ${GLOBAL_INCLUDES})
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME}
        "${PROJECT_SOURCE_DIR}/dependencies/sdl/lib/libSDL2main.a"
        "${PROJECT_SOURCE_DIR}/dependencies/sdl/lib/libSDL2.dll.a")
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
