cmake_minimum_required(VERSION 3.21.0)

project(Tephra VERSION 0.1.0)

find_package(Vulkan REQUIRED)

if(TEPHRA_USE_PLATFORM_XLIB)
    find_package(X11 REQUIRED)
endif()

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
    add_library(Tephra STATIC ${TEPHRA_SOURCES})
    target_compile_definitions(Tephra PUBLIC TEPHRA_STATIC_BUILD)
    set_target_properties(Tephra PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
    add_library(Tephra SHARED ${TEPHRA_SOURCES})
    target_compile_definitions(Tephra PUBLIC TEPHRA_SHARED_BUILD)
endif()

set_target_properties(Tephra PROPERTIES PREFIX "" OUTPUT_NAME "tephra")

target_compile_definitions(Tephra
    PUBLIC
        VK_NO_PROTOTYPES
        TEPHRA_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        TEPHRA_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        TEPHRA_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_link_libraries(Tephra
    PUBLIC
        CaptalFoundation
        NotEnoughStandards::NotEnoughStandards
        Vulkan::Headers
    PRIVATE
        X11::X11
)

target_compile_definitions(Tephra
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

target_include_directories(Tephra
    SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/src/tephra/stb
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_sources(Tephra PRIVATE src/tephra/resources.rc)
    target_compile_definitions(Tephra PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(CAPTAL_BUILD_TEPHRA_EXAMPLES)
    add_executable(TephraExample "example.cpp")
    target_link_libraries(TephraExample PRIVATE Tephra)
endif()

if(CAPTAL_BUILD_TEPHRA_TESTS)
    add_executable(TephraTest "test.cpp")
    target_link_libraries(TephraTest PRIVATE Tephra Apyre Catch2)
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/tephra
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp)
