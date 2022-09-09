# Download and build external project SDL2

option(CAPTAL_SUPERBUILD_EXCLUDE_SDL2 "Does not build SDL2 as a part of the superbuild, falling back on classic find_package." OFF)

if(NOT CAPTAL_SUPERBUILD_EXCLUDE_SDL2)
    set(CAPTAL_SUPERBUILD_SDL2_GIT_URL "https://github.com/libsdl-org/SDL.git" CACHE STRING "Used url for SDL2 git clone (allow usage of mirrors or interal repo)")
    set(CAPTAL_SUPERBUILD_SDL2_GIT_TAG "release-2.0.20" CACHE STRING "Used tag for SDL2 git clone")

    mark_as_advanced(CAPTAL_SUPERBUILD_SDL2_GIT_URL)
    mark_as_advanced(CAPTAL_SUPERBUILD_SDL2_GIT_TAG)

    ExternalProject_Add(SDL2
        GIT_REPOSITORY ${CAPTAL_SUPERBUILD_SDL2_GIT_URL}
        GIT_TAG        ${CAPTAL_SUPERBUILD_SDL2_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        PREFIX         "${CMAKE_BINARY_DIR}/dependencies/sdl"
        CMAKE_ARGS
            "-Wno-dev"

            "-DSDL_AUDIO=OFF"
            "-DSDL_FILESYSTEM=OFF"
            "-DSDL_RENDER=OFF"

            "-DSDL_SHARED=OFF"
            "-DSDL_STATIC=ON"
            "-DSDL_STATIC_PIC=ON"
            "-DSDL_TEST=OFF"

            "-DSDL_DIRECTFB=OFF"
            "-DSDL_DIRECTFB_SHARED=OFF"
            "-DSDL_OPENGL=OFF"
            "-DSDL_OPENGLES=OFF"
            "-DSDL_PTHREADS=OFF"
            "-DSDL_PTHREADS_SEM=OFF"
            "-DSDL_RPATH=OFF"
            "-DSDL_CLOCK_GETTIME=OFF"
            "-DSDL_DIRECTX=OFF"
            "-DSDL_RENDER_D3D=OFF"
            "-DSDL_RENDER_METAL=OFF"
            "-DSDL_VIVANTE=OFF"
            "-DSDL_VULKAN=ON"
            "-DSDL_METAL=OFF"
            "-DSDL_KMSDRM=OFF"
            "-DSDL_KMSDRM_SHARED=OFF"
            "-DSDL_OFFSCREEN=OFF"
            "-DSDL_HIDAPI=OFF"
            "-DSDL_VIRTUAL_JOYSTICK=OFF"

            "-DCMAKE_C_FLAGS=${EXTERNAL_FLAGS}"
            "-DCMAKE_C_FLAGS_DEBUG=${EXTERNAL_FLAGS_DEBUG}"
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=${EXTERNAL_FLAGS_RELWITHDEBINFO}"
            "-DCMAKE_C_FLAGS_RELEASE=${EXTERNAL_FLAGS_RELEASE}"
            "-DCMAKE_C_FLAGS_MINSIZEREL=${EXTERNAL_FLAGS_MINSIZEREL}"

            "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies/sdl/install"
            "-DCMAKE_INSTALL_MESSAGE=LAZY"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    )

    list(APPEND ADDITIONAL_CMAKE_ARGS "-DSDL2_DIR:PATH=${CMAKE_BINARY_DIR}/dependencies/sdl/install/lib/cmake/SDL2")
    list(APPEND DEPENDENCIES "SDL2")
endif()
