cmake_minimum_required(VERSION 3.15.7)

project(tephra VERSION 0.1.0)

option(TEPHRA_USE_PLATFORM_ANDROID "If on, enable the Android surface extension" OFF)
option(TEPHRA_USE_PLATFORM_IOS "If on, enable the IOS surface extension" OFF)
option(TEPHRA_USE_PLATFORM_WIN32 "If on, enable the Windows surface extension" OFF)
option(TEPHRA_USE_PLATFORM_MACOS "If on, enable the OSX surface extension" OFF)
option(TEPHRA_USE_PLATFORM_XLIB "If on, enable the Linux Xlib surface extension" OFF)
option(TEPHRA_USE_PLATFORM_XCB "If on, enable the Linux XCB surface extension" OFF)
option(TEPHRA_USE_PLATFORM_WAYLAND "If on, enable the Linux Wayland surface extension" OFF)

find_package(Vulkan REQUIRED)
captal_download_submodule(tephra/external/stb FALSE)

set(TEPHRA_SOURCES
    #Headers:
    src/tephra/config.hpp
    src/tephra/enumerations.hpp
    src/tephra/hardware.hpp
    src/tephra/surface.hpp
    src/tephra/swapchain.hpp
    src/tephra/application.hpp
    src/tephra/debug_utils.hpp
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
    src/tephra/query.hpp

    src/tephra/vulkan/vulkan.hpp
    src/tephra/vulkan/vulkan_functions.hpp
    src/tephra/vulkan/memory.hpp
    src/tephra/vulkan/helper.hpp

    #Sources:
    src/tephra/hardware.cpp
    src/tephra/surface.cpp
    src/tephra/swapchain.cpp
    src/tephra/application.cpp
    src/tephra/debug_utils.cpp
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
    src/tephra/query.cpp

    src/tephra/vulkan/vulkan.cpp
    src/tephra/vulkan/vulkan_functions.cpp
    src/tephra/vulkan/memory.cpp
    src/tephra/vulkan/helper.cpp
)

if(CAPTAL_BUILD_TEPHRA_STATIC)
    add_library(tephra STATIC ${TEPHRA_SOURCES})
    target_compile_definitions(tephra PUBLIC TEPHRA_STATIC_BUILD)
    set(TO_EXPORT ${TO_EXPORT} tephra not_enough_standards PARENT_SCOPE)
else()
    add_library(tephra SHARED ${TEPHRA_SOURCES})
    target_compile_definitions(tephra PUBLIC TEPHRA_SHARED_BUILD)
    set(TO_EXPORT ${TO_EXPORT} tephra PARENT_SCOPE)
endif()

set_target_properties(tephra PROPERTIES PREFIX "")

target_compile_definitions(tephra
    PUBLIC
        VK_NO_PROTOTYPES
        TEPHRA_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        TEPHRA_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        TEPHRA_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_link_libraries(tephra
    PRIVATE
        not_enough_standards
    PUBLIC
        captal_foundation
)

target_compile_definitions(tephra
    PUBLIC
        $<$<BOOL:${TEPHRA_USE_PLATFORM_ANDROID}>:TPH_PLATFORM_ANDROID>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_IOS}>:TPH_PLATFORM_IOS>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_WIN32}>:TPH_PLATFORM_WIN32>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_MACOS}>:TPH_PLATFORM_MACOS>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_XLIB}>:TPH_PLATFORM_XLIB>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_XCB}>:TPH_PLATFORM_XCB>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_WAYLAND}>:TPH_PLATFORM_WAYLAND>
    PRIVATE
        $<$<BOOL:${TEPHRA_USE_PLATFORM_ANDROID}>:VK_USE_PLATFORM_ANDROID_KHR>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_IOS}>:VK_USE_PLATFORM_IOS_MVK>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_WIN32}>:VK_USE_PLATFORM_WIN32_KHR>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_MACOS}>:VK_USE_PLATFORM_MACOS_MVK>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_XLIB}>:VK_USE_PLATFORM_XLIB_KHR>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_XCB}>:VK_USE_PLATFORM_XCB_KHR>
        $<$<BOOL:${TEPHRA_USE_PLATFORM_WAYLAND}>:VK_USE_PLATFORM_WAYLAND_KHR>
)

target_include_directories(tephra
    SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/external/stb
    PUBLIC
        ${Vulkan_INCLUDE_DIRS}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

if(WIN32)
    target_sources(tephra PRIVATE src/tephra/resources.rc)
endif()

if(CAPTAL_BUILD_TEPHRA_EXAMPLES)
    add_executable(tephra_example "example.cpp")
    target_link_libraries(tephra_example PRIVATE tephra)

    install(TARGETS tephra tephra_example
            RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/example
            ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/example
            COMPONENT executable)
endif()

if(CAPTAL_BUILD_TEPHRA_TESTS)
    add_executable(tephra_test "test.cpp")
    target_link_libraries(tephra_test PRIVATE tephra apyre Catch2)
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

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/tephra
        DESTINATION ${PROJECT_SOURCE_DIR}/include
        FILES_MATCHING PATTERN *.hpp)
