
set(PA_BUILD_EXAMPLES      OFF CACHE INTERNAL "")
set(PA_BUILD_SHARED        OFF CACHE INTERNAL "")
set(PA_BUILD_STATIC        ON  CACHE INTERNAL "")
set(PA_BUILD_TESTS         OFF CACHE INTERNAL "")
set(PA_DISABLE_INSTALL     ON  CACHE INTERNAL "")
set(PA_ENABLE_DEBUG_OUTPUT OFF CACHE INTERNAL "")
set(PA_LIBNAME_ADD_SUFFIX  OFF CACHE INTERNAL "")
set(PA_UNICODE_BUILD       ON  CACHE INTERNAL "")

add_subdirectory(external/portaudio EXCLUDE_FROM_ALL)

unset(PA_BUILD_EXAMPLES      CACHE)
unset(PA_BUILD_SHARED        CACHE)
unset(PA_BUILD_STATIC        CACHE)
unset(PA_BUILD_TESTS         CACHE)
unset(PA_DISABLE_INSTALL     CACHE)
unset(PA_ENABLE_DEBUG_OUTPUT CACHE)
unset(PA_LIBNAME_ADD_SUFFIX  CACHE)
unset(PA_UNICODE_BUILD       CACHE)