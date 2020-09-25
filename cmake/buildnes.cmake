set(NES_BUILD_EXAMPLES OFF CACHE INTERNAL "")

add_subdirectory(external/nes external/nes/bin EXCLUDE_FROM_ALL)

unset(NES_BUILD_EXAMPLES CACHE)
