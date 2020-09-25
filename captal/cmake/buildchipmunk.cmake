
set(LIB_INSTALL_DIR    OFF CACHE INTERNAL "")
set(BIN_INSTALL_DIR    OFF CACHE INTERNAL "")
set(BUILD_DEMOS        OFF CACHE INTERNAL "")
set(INSTALL_DEMOS      OFF CACHE INTERNAL "")
set(BUILD_SHARED       OFF CACHE INTERNAL "")
set(BUILD_STATIC       ON  CACHE INTERNAL "")
set(INSTALL_STATIC     OFF CACHE INTERNAL "")
set(FORCE_CLANG_BLOCKS OFF CACHE INTERNAL "")

add_compile_definitions(CP_COLLISION_TYPE_TYPE=uint64_t
                        CP_GROUP_TYPE=uint64_t
                        CP_BITMASK_TYPE=uint64_t
                        CP_TIMESTAMP_TYPE=uint64_t)

add_subdirectory(external/chipmunk)

unset(LIB_INSTALL_DIR    CACHE)
unset(BIN_INSTALL_DIR    CACHE)
unset(BUILD_DEMOS        CACHE)
unset(INSTALL_DEMOS      CACHE)
unset(BUILD_SHARED       CACHE)
unset(BUILD_STATIC       CACHE)
unset(INSTALL_STATIC     CACHE)
unset(FORCE_CLANG_BLOCKS CACHE)
