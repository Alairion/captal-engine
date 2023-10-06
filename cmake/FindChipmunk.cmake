find_library(CHIPMUNK_LIBS
    NAMES
        chipmunk
    PATH_SUFFIXES
        lib
        OFF
    PATHS
        ${Chipmunk_DIR}
)

find_path(CHIPMUNK_INCLUDE_DIRS
    NAMES
        chipmunk.h
    PATH_SUFFIXES
        chipmunk
    PATHS
        ${Chipmunk_DIR}
        ${Chipmunk_DIR}/include
)

add_library(chipmunk::chipmunk_static STATIC IMPORTED)
set_target_properties(chipmunk::chipmunk_static PROPERTIES
    IMPORTED_LOCATION ${CHIPMUNK_LIBS}
    INTERFACE_INCLUDE_DIRECTORIES ${CHIPMUNK_INCLUDE_DIRS}
    )
