
set(BUILD_SHARED_LIBS                    OFF CACHE INTERNAL "")
set(FT_WITH_ZLIB                         OFF CACHE INTERNAL "")
set(FT_WITH_BZIP2                        OFF CACHE INTERNAL "")
set(FT_WITH_PNG                          OFF CACHE INTERNAL "")
set(FT_WITH_HARFBUZZ                     OFF CACHE INTERNAL "")
set(FT_WITH_BROTLI                       OFF CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB      OFF CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2     OFF CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_PNG       OFF CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz  OFF CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec OFF CACHE INTERNAL "")

add_subdirectory(external/freetype EXCLUDE_FROM_ALL)

unset(BUILD_SHARED_LIBS                    CACHE)
unset(FT_WITH_ZLIB                         CACHE)
unset(FT_WITH_BZIP2                        CACHE)
unset(FT_WITH_PNG                          CACHE)
unset(FT_WITH_HARFBUZZ                     CACHE)
unset(FT_WITH_BROTLI                       CACHE)
unset(CMAKE_DISABLE_FIND_PACKAGE_ZLIB      CACHE)
unset(CMAKE_DISABLE_FIND_PACKAGE_BZip2     CACHE)
unset(CMAKE_DISABLE_FIND_PACKAGE_PNG       CACHE)
unset(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz  CACHE)
unset(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec CACHE)
