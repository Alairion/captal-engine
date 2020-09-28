
captal_download_submodule(captal/external/zlib TRUE)

file(REMOVE external/zlib/CMakeLists.txt)
file(COPY ${PROJECT_SOURCE_DIR}/cmake/zlib/CMakeLists.txt DESTINATION ${PROJECT_SOURCE_DIR}/external/zlib)

add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
