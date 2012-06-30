# - Find ALLEGRO
# Find the ALLEGRO includes and libraries
#
#  ALLEGRO_INCLUDE_DIR - where to find ALLEGRO headers.
#  ALLEGRO_LIBRARIES   - List of libraries when using Allegro.
#  ALLEGRO_FOUND       - True if libOgreMain found.

if(ALLEGRO_INCLUDE_DIR)
    # Already in cache, be silent
    set(ALLEGRO_FIND_QUIETLY TRUE)
endif(ALLEGRO_INCLUDE_DIR)

find_path(ALLEGRO_INCLUDE_DIR allegro5/allegro.h)

find_library(ALLEGRO_LIBRARY NAMES allegro)
find_library(ALLEGRO_IMAGE_LIBRARY NAMES allegro_image)
find_library(ALLEGRO_FONT_LIBRARY NAMES allegro_font)
find_library(ALLEGRO_PRIMITIVES_LIBRARY NAMES allegro_primitives)

# Handle the QUIETLY and REQUIRED arguments and set ALLEGRO_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ALLEGRO DEFAULT_MSG
    ALLEGRO_INCLUDE_DIR ALLEGRO_LIBRARY ALLEGRO_IMAGE_LIBRARY
    ALLEGRO_FONT_LIBRARY ALLEGRO_PRIMITIVES_LIBRARY)

if(ALLEGRO_FOUND)
    list(APPEND ALLEGRO_LIBRARIES
        ${ALLEGRO_LIBRARY} ${ALLEGRO_IMAGE_LIBRARY}
        ${ALLEGRO_FONT_LIBRARY} ${ALLEGRO_PRIMITIVES_LIBRARY}
        )
else(ALLEGRO_FOUND)
    set(ALLEGRO_LIBRARIES)
endif(ALLEGRO_FOUND)

mark_as_advanced(ALLEGRO_INCLUDE_DIR ALLEGRO_LIBRARY ALLEGRO_IMAGE_LIBRARY)
