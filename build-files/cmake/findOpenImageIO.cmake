#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
#
#    SORT is a free software written for educational purpose. Anyone can distribute
#    or modify it under the the terms of the GNU General Public License Version 3 as
#    published by the Free Software Foundation. However, there is NO warranty that
#    all components are functional in a perfect manner. Without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with
#    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
#

# Module to find OpenImageIO.
#
# This module will set
#   OPENIMAGEIO_FOUND           true, if found
#   OPENIMAGEIO_ROOT_DIR        root directory of Open ImageIO library
#   OPENIMAGEIO_INCLUDE_DIR     directory where headers are found
#   OPENIMAGEIO_LIBRARIES       libraries for OpenEXR + IlmBase
#	OPENIMAGEIO_DLL				Dlls for OpenImageIO

# Notification of what is being done.
if (NOT OpenImageIO_FIND_QUIETLY)
    message( STATUS "Finding OpenImageIO." )
endif ()

if( SORT_PLATFORM_WIN OR SORT_PLATFORM_MAC )
	set( OPENIMAGEIO_ROOT_DIR ${SORT_SOURCE_DIR}/dependencies/oiio )
	if( EXISTS ${OPENIMAGEIO_ROOT_DIR})
		set( OPENIMAGEIO_FOUND TRUE )
		set( OPENIMAGEIO_INCLUDE_DIR ${OPENIMAGEIO_ROOT_DIR}/include )
		set( OPENIMAGEIO_LIBRARY_DIR ${OPENIMAGEIO_ROOT_DIR}/lib )
		if( SORT_PLATFORM_WIN )
			file( GLOB_RECURSE OPENIMAGEIO_LIBRARIES ${OPENIMAGEIO_LIBRARY_DIR}/*.lib )
		elseif( SORT_PLATFORM_MAC )
			file( GLOB_RECURSE OPENIMAGEIO_LIBRARIES ${OPENIMAGEIO_LIBRARY_DIR}/*.dylib )
		else()
			# reserved for ubuntu
		endif()

		if( SORT_PLATFORM_WIN )
			file( GLOB_RECURSE OPENIMAGEIO_DLL ${OPENIMAGEIO_ROOT_DIR}/bin/*.dll )
		endif( SORT_PLATFORM_WIN )
	else()
		set( OPENIMAGEIO_FOUND FALSE )
	endif()
else()
	# Add the include folder in include directories
	#include_directories( ${OIIO_INCLUDE_DIR} )

	# On linux, OpenImageIO is packed in the dependencies
	if (SORT_PLATFORM_LINUX)
		set( OPENIMAGEIO_ROOT_DIR "${SORT_SOURCE_DIR}/dependencies/oiio" )
	endif()

	# If 'OPENIMAGE_HOME' not set, use the env variable of that name if available
	if (NOT OPENIMAGEIO_ROOT_DIR AND NOT $ENV{OPENIMAGEIO_ROOT_DIR} STREQUAL "")
		set (OPENIMAGEIO_ROOT_DIR $ENV{OPENIMAGEIO_ROOT_DIR})
	endif ()

	if (SORT_PLATFORM_MAC)
	find_library ( OPENIMAGEIO_LIBRARY
				   NAMES OpenImageIO
				   HINTS ${OPENIMAGEIO_ROOT_DIR}/lib
				   PATH_SUFFIXES lib64 lib so
				   PATHS "${OPENIMAGEIO_ROOT_DIR}/lib" )
	find_path ( OPENIMAGEIO_INCLUDE_DIR
				NAMES OpenImageIO/imageio.h
				HINTS ${OPENIMAGEIO_ROOT_DIR}/include
				PATH_SUFFIXES include )
	find_program ( OIIOTOOL_BIN
				   NAMES oiiotool oiiotool.exe
				   HINTS ${OPENIMAGEIO_ROOT_DIR}/bin
				   PATH_SUFFIXES bin )
	elseif(SORT_PLATFORM_LINUX)
	set( OPENIMAGEIO_INCLUDE_DIR ${OPENIMAGEIO_ROOT_DIR}/include )
	set( OPENIMAGEIO_LIBRARY_DIR ${OPENIMAGEIO_ROOT_DIR}/lib )
	#file( GLOB_RECURSE OPENIMAGEIO_LIBRARIES ${OPENIMAGEIO_LIBRARY_DIR}/*.so )
	set( OPENIMAGEIO_LIBRARIES ${OPENIMAGEIO_LIBRARY_DIR}/libOpenImageIO.so )
	else()
	endif()

	# need to clean this later.
	if (SORT_PLATFORM_MAC)
		set ( OPENIMAGEIO_LIBRARIES ${OPENIMAGEIO_LIBRARY} )
		get_filename_component (OPENIMAGEIO_LIBRARY_DIRS "${OPENIMAGEIO_LIBRARY}" DIRECTORY CACHE)
	endif()

	include (FindPackageHandleStandardArgs)
	find_package_handle_standard_args (OpenImageIO
		FOUND_VAR     OPENIMAGEIO_FOUND
		REQUIRED_VARS OPENIMAGEIO_INCLUDE_DIR OPENIMAGEIO_LIBRARIES
					  OPENIMAGEIO_LIBRARY_DIRS OPENIMAGEIO_VERSION
		VERSION_VAR   OPENIMAGEIO_VERSION
	)
endif()

# Try to figure out version number
set (OIIO_VERSION_HEADER "${OPENIMAGEIO_INCLUDE_DIR}/OpenImageIO/oiioversion.h")
if (EXISTS "${OIIO_VERSION_HEADER}")
	file (STRINGS "${OIIO_VERSION_HEADER}" TMP REGEX "^#define OIIO_VERSION_MAJOR .*$")
	string (REGEX MATCHALL "[0-9]+" OPENIMAGEIO_VERSION_MAJOR ${TMP})
	file (STRINGS "${OIIO_VERSION_HEADER}" TMP REGEX "^#define OIIO_VERSION_MINOR .*$")
	string (REGEX MATCHALL "[0-9]+" OPENIMAGEIO_VERSION_MINOR ${TMP})
	file (STRINGS "${OIIO_VERSION_HEADER}" TMP REGEX "^#define OIIO_VERSION_PATCH .*$")
	string (REGEX MATCHALL "[0-9]+" OPENIMAGEIO_VERSION_PATCH ${TMP})
	set (OPENIMAGEIO_VERSION "${OPENIMAGEIO_VERSION_MAJOR}.${OPENIMAGEIO_VERSION_MINOR}.${OPENIMAGEIO_VERSION_PATCH}")
endif ()

if(NOT OpenImageIO_FIND_QUIETLY)
    message( STATUS "OPENIMAGEIO Found        = ${OPENIMAGEIO_FOUND}")
    message( STATUS "OpenImageIO Version      = ${OPENIMAGEIO_VERSION}" )
    message( STATUS "OPENIMAGEIO_ROOT_DIR     = ${OPENIMAGEIO_ROOT_DIR}" )
    message( STATUS "OpenImageIO includes     = ${OPENIMAGEIO_INCLUDE_DIR}" )
    message( STATUS "OpenImageIO library_dirs = ${OPENIMAGEIO_LIBRARY_DIR}" )
    message( STATUS "OpenImageIO libraries    = ${OPENIMAGEIO_LIBRARIES}" )
    if( SORT_PLATFORM_WIN )
		message( STATUS "OpenImageIO DLLs         = ${OPENIMAGEIO_DLL}")
	endif( SORT_PLATFORM_WIN )
endif()

mark_as_advanced (
    OPENIMAGEIO_INCLUDE_DIR
    OPENIMAGEIO_LIBRARIES
    OPENIMAGEIO_LIBRARY_DIRS
    OPENIMAGEIO_VERSION
    OPENIMAGEIO_VERSION_MAJOR
    OPENIMAGEIO_VERSION_MINOR
    OPENIMAGEIO_VERSION_PATCH
    OPENIMAGEIO_LIBRARY
)

# Adding include directory
include_directories( ${OPENIMAGEIO_INCLUDE_DIR} )

# Copy the DLL to bin folder
if( SORT_PLATFORM_WIN )
	file(COPY ${OPENIMAGEIO_DLL} DESTINATION ${SORT_SOURCE_DIR}/bin/ )
endif( SORT_PLATFORM_WIN )
