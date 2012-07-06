# Build script for nostos
# ========================

cmake_minimum_required(VERSION 2.8.1)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

project(nostos C)

set(NOSTOS_VERSION_MAJOR 0)
set(NOSTOS_VERSION_MINOR 1)
set(NOSTOS_VERSION_PATCH 0)
set(PROJECT_VERSION ${NOSTOS_VERSION_MAJOR}.${NOSTOS_VERSION_MINOR}.${NOSTOS_VERSION_PATCH})

# Search in the 'cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Environment tests
# =================

#None

# Build options
# =============

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

add_definitions(-std=c99)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
#option(BUILD_SAMPLES "Build samples" OFF)

#if(NOT BUILD_SHARED_LIBS)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
#endif()

#set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_C_FLAGS -Wall)

# Find dependencies
# =================

find_package(Allegro REQUIRED)
include_directories(${ALLEGRO_INCLUDE_DIR})
list(APPEND LINK_LIBS ${ALLEGRO_LIBRARY})
list(APPEND LINK_LIBS ${ALLEGRO_IMAGE_LIBRARY})
list(APPEND LINK_LIBS ${ALLEGRO_FONT_LIBRARY})
list(APPEND LINK_LIBS ${ALLEGRO_PRIMITIVES_LIBRARY})

find_package(LIBXML2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
list(APPEND LINK_LIBS ${LIBXML2_LIBRARIES})

list(APPEND LINK_LIBS m)

# Setup paths
# ===========

list(APPEND NOSTOS_SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/aabbtree.c
    ${PROJECT_SOURCE_DIR}/src/aatree.c
    ${PROJECT_SOURCE_DIR}/src/box.c
    ${PROJECT_SOURCE_DIR}/src/game.c
    ${PROJECT_SOURCE_DIR}/src/screen.c
    ${PROJECT_SOURCE_DIR}/src/sprite.c
    ${PROJECT_SOURCE_DIR}/src/tiled.c
    ${PROJECT_SOURCE_DIR}/src/utils.c
    ${PROJECT_SOURCE_DIR}/src/vector2d.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)
list(APPEND NOSTOS_HDR_FILES
    ${PROJECT_SOURCE_DIR}/src/aabbtree.h
    ${PROJECT_SOURCE_DIR}/src/aatree.h
    ${PROJECT_SOURCE_DIR}/src/box.h
    ${PROJECT_SOURCE_DIR}/src/game.h
    ${PROJECT_SOURCE_DIR}/src/screen.h
    ${PROJECT_SOURCE_DIR}/src/sprite.h
    ${PROJECT_SOURCE_DIR}/src/tiled.h
    ${PROJECT_SOURCE_DIR}/src/utils.h
    ${PROJECT_SOURCE_DIR}/src/vector2d.h
)


# Build Example
# =========

add_executable(nostos
    ${NOSTOS_SRC_FILES}
    ${NOSTOS_HDR_FILES}
)

set_target_properties(nostos PROPERTIES
    VERSION ${PROJECT_VERSION}
#    LINK_SEARCH_START_STATIC on
)

install(TARGETS nostos
    RUNTIME DESTINATION bin
)

# Link libraries
# ==============

target_link_libraries(nostos ${LINK_LIBS})


# Installation
# ============

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/nostos
        DESTINATION include
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/bin
        DESTINATION bin
)
