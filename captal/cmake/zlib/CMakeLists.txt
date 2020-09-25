cmake_minimum_required(VERSION 3.0)

project(zlib VERSION 1.2)

configure_file(zconf.h.cmakein zconf.h @ONLY)

#Zlib CMakeLists.txt is one of the worst i have ever seen, we won't use it
add_library(zlib_static STATIC
    zconf.h
    zlib.h
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h

    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

target_compile_definitions(zlib_static PUBLIC ZLIB_CONST)

target_include_directories(zlib_static
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)