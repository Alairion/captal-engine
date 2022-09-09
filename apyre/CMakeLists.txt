cmake_minimum_required(VERSION 3.21.0)

project(Apyre VERSION 0.1.0)

find_package(SDL2 REQUIRED)

set(APYRE_SOURCES
    #Headers:
    src/apyre/config.hpp
    src/apyre/application.hpp
    src/apyre/monitor.hpp
    src/apyre/inputs.hpp
    src/apyre/window.hpp
    src/apyre/event.hpp
    src/apyre/messagebox.hpp
    src/apyre/power.hpp

    #Sources:
    src/apyre/application.cpp
    src/apyre/inputs.cpp
    src/apyre/window.cpp
    src/apyre/event.cpp
    src/apyre/messagebox.cpp
    src/apyre/power.cpp
)

if(CAPTAL_BUILD_APYRE_STATIC)
    add_library(Apyre STATIC ${APYRE_SOURCES})
    target_compile_definitions(Apyre PUBLIC APYRE_STATIC_BUILD)
else()
    add_library(Apyre SHARED ${APYRE_SOURCES})
    target_compile_definitions(Apyre PUBLIC APYRE_SHARED_BUILD)
    set_target_properties(Apyre PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set_target_properties(Apyre PROPERTIES PREFIX "" OUTPUT_NAME "apyre")

target_compile_definitions(Apyre
    PUBLIC
        APYRE_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
        APYRE_MINOR_VERSION=${PROJECT_VERSION_MINOR}
        APYRE_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)

target_include_directories(Apyre
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/apyre>
)

target_link_libraries(Apyre
    PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
    PUBLIC
        CaptalFoundation
)

if(WIN32)
    target_sources(Apyre PRIVATE src/apyre/resources.rc)
    target_link_libraries(Apyre PRIVATE dwmapi)
    target_compile_definitions(Apyre PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(CAPTAL_BUILD_APYRE_EXAMPLES)
    add_executable(ApyreTest "main.cpp")
    target_link_libraries(ApyreTest PRIVATE Apyre)
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/apyre
        DESTINATION include
        FILES_MATCHING PATTERN *.hpp)
