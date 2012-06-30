# - Find LIBXML2
# Find the LIBXML2 includes and libraries
#
#  LIBXML2_INCLUDE_DIR - where to find LIBXML2 headers.
#  LIBXML2_LIBRARIES   - List of libraries when using Yaml.
#  LIBXML2_FOUND       - True if libOgreMain found.

if(LIBXML2_INCLUDE_DIR)
    # Already in cache, be silent
    set(LIBXML2_FIND_QUIETLY TRUE)
endif()

find_path(LIBXML2_INCLUDE_DIR libxml2/libxml/xpath.h PATHS
    ${LIBXML2_HOME}/include
    $ENV{LIBXML2_HOME}/include
    /usr/local/include
    /usr/include
    NO_DEFAULT_PATH
    )

if(LIBXML2_INCLUDE_DIR)
    set(LIBXML2_INCLUDE_DIR "${LIBXML2_INCLUDE_DIR}/libxml2")
else()
    find_path(LIBXML2_INCLUDE_DIR libxml/xpath.h
        ${LIBXML2_HOME}/include
        $ENV{LIBXML2_HOME}/include)
endif()

find_library(LIBXML2_LIBRARIES xml2 libxml2 PATHS
    ${LIBXML2_HOME}/lib
    $ENV{LIBXML2_HOME}/lib
    /usr/local/lib
    /usr/lib
    NO_DEFAULT_PATH
    )

# Handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBXML2 DEFAULT_MSG
    LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES)

mark_as_advanced(
    LIBXML2_INCLUDE_DIR 
    LIBXML2_LIBRARIES 
    LIBXML2_FOUND
    )
