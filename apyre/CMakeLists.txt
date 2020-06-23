cmake_minimum_required(VERSION 3.0.0)

project(apyre VERSION 0.1.0)

option(APYRE_BUILD_EXAMPLE ON)

set(SDL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include/sdl")
set(SDL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external/libSDL2main.a" "${PROJECT_SOURCE_DIR}/../libs/external/libSDL2-static.a")

add_library(apyre SHARED
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
    SYSTEM PRIVATE
        ${SDL_INCLUDES}
        ${GLOBAL_INCLUDES}
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(apyre
    PRIVATE
        ${SDL_LIBRARIES}
    PUBLIC
        captal_foundation
)

if(WIN32)
    target_sources(apyre PRIVATE src/apyre/resources.rc)
endif()

if(MINGW)
    target_link_libraries(apyre
        PRIVATE
            imagehlp
            dinput8
            dxguid
            dxerr8
            user32
            gdi32
            winmm
            imm32
            ole32
            oleaut32
            shell32
            version
            uuid
    )
endif()

if(APYRE_BUILD_EXAMPLE)
    add_executable(apyre_test "main.cpp")
    target_link_libraries(apyre_test PRIVATE apyre)
    target_include_directories(apyre_test PRIVATE ${GLOBAL_INCLUDES})
endif()

install(TARGETS apyre
        CONFIGURATIONS Debug
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/debug"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/debug"
        COMPONENT library)

install(TARGETS apyre
        CONFIGURATIONS Release
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../test/release"
        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../libs/release"
        COMPONENT library)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/include/apyre"
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" DESTINATION "${PROJECT_SOURCE_DIR}/../libs/include/apyre"
        FILES_MATCHING PATTERN "*.hpp")
