
file(COPY cmake/zlib/CMakeLists.txt DESTINATION external/zlib)

captal_download_submodule(captal/external/zlib)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
