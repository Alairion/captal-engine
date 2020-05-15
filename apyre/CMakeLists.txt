cmake_minimum_required(VERSION 3.0.0)

project(apyre)

option(APYRE_BUILD_EXAMPLE OFF)

set(GLOBAL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include")
set(GLOBAL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external")

set(SDL_INCLUDES "${PROJECT_SOURCE_DIR}/../libs/include/sdl")
set(SDL_LIBRARIES "${PROJECT_SOURCE_DIR}/../libs/external/libSDL2main.a"
                  "${PROJECT_SOURCE_DIR}/../libs/external/libSDL2-static.a")

if(MINGW)
    set(SDL_LIBRARIES ${SDL_LIBRARIES} imagehlp dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid)
endif()

set(HEADERS
    "src/config.hpp"
    "src/application.hpp"
    "src/monitor.hpp"
    "src/window.hpp"
    "src/event.hpp")

set(SOURCES
    "src/application.cpp"
    "src/window.cpp"
    "src/event.cpp")

add_definitions(-DSDL_MAIN_HANDLED -DAPYRE_SHARED_BUILD)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL_INCLUDES} ${GLOBAL_INCLUDES})
target_link_libraries(${PROJECT_NAME} ${SDL_LIBRARIES})

if(APYRE_BUILD_EXAMPLE)
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
