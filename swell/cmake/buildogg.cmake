set(BUILD_SHARED_LIBS            OFF CACHE INTERNAL "")
set(BUILD_FRAMEWORK              OFF CACHE INTERNAL "")
set(BUILD_TESTING                OFF CACHE INTERNAL "")
set(INSTALL_DOCS                 OFF CACHE INTERNAL "")
set(INSTALL_PKG_CONFIG_MODULE    OFF CACHE INTERNAL "")
set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE INTERNAL "")

captal_download_submodule(swell/external/ogg TRUE)
add_subdirectory(external/ogg EXCLUDE_FROM_ALL)
add_library(Ogg::ogg ALIAS ogg)

unset(BUILD_SHARED_LIBS            CACHE)
unset(BUILD_FRAMEWORK              CACHE)
unset(BUILD_TESTING                CACHE)
unset(INSTALL_DOCS                 CACHE)
unset(INSTALL_PKG_CONFIG_MODULE    CACHE)
unset(INSTALL_CMAKE_PACKAGE_MODULE CACHE)
