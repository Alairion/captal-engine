set(NES_BUILD_EXAMPLES OFF CACHE INTERNAL "")

captal_download_submodule(external/nes)
add_subdirectory(external/nes external/nes/bin EXCLUDE_FROM_ALL)

unset(NES_BUILD_EXAMPLES CACHE)
