
file(COPY cmake/zlib/CMakeLists.txt DESTINATION external/zlib)

captal_download_submodule(captal/external/zlib TRUE)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
