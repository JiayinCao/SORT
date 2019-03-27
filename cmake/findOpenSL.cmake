#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
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

# Module to find OpenShadingLanguage.
#
# This module will set
#   OSL_FOUND          true, if found
#   OSL_ROOT_DIR       root directory of Open Shading Language
#   OSL_INCLUDE_DIR    directory where headers are found
#   OSL_LIBRARIES      libraries for OpenEXR + IlmBase
#   OSL_DLL            Library DLLs

# Notification of what is being done.
if (NOT OpenSL_FIND_QUIETLY)
    message( STATUS "Finding OpenShadingLanguage." )
endif ()

set( OSL_ROOT_DIR ${SORT_SOURCE_DIR}/dependencies/osl )
if( EXISTS ${OSL_ROOT_DIR})
	set( OSL_FOUND TRUE )
    set( OSL_INCLUDE_DIR ${OSL_ROOT_DIR}/include )
    set( OSL_LIBRARY_DIR ${OSL_ROOT_DIR}/lib )
if( SORT_PLATFORM_MAC )
    file( GLOB_RECURSE OSL_LIBRARIES ${OSL_LIBRARY_DIR}/*.dylib )
elseif( SORT_PLATFORM_LINUX )
    file( GLOB_RECURSE OSL_LIBRARIES ${OSL_LIBRARY_DIR}/*.so )
else() # SORT_PLATFORM_WIN
    file( GLOB_RECURSE OSL_LIBRARIES ${OSL_LIBRARY_DIR}/*.lib )
    file( GLOB_RECURSE OSL_DLL  ${OSL_ROOT_DIR}/bin/*.dll )
endif()
else()
	set( OSL_FOUND FALSE )
endif()

# Hide the options in UI
MARK_AS_ADVANCED(
    OSL_FOUND
    OSL_ROOT_DIR
    OSL_INCLUDE_DIR
    OSL_LIBRARIES
)

# Output finding result
if (NOT OpenSL_FIND_QUIETLY)
	message( STATUS "OpenSL Found             = ${OSL_FOUND}")
	message( STATUS "OSL_ROOT_DIR             = ${OSL_ROOT_DIR}")
	message( STATUS "OSL_INCLUDE_DIR          = ${OSL_INCLUDE_DIR}")
    message( STATUS "OSL_LIBRARY_DIR          = ${OSL_LIBRARY_DIR}")
	message( STATUS "OSL_LIBRARIES            = ${OSL_LIBRARIES}")
    if(SORT_PLATFORM_WIN)
        message( STATUS "OSL_DLL                  = ${OSL_DLL}")
    endif(SORT_PLATFORM_WIN)
endif()

# Add the include folder in include directories
include_directories( "${OSL_INCLUDE_DIR}" )

# Copy the DLL to bin folder
if( SORT_PLATFORM_WIN )
    file(COPY ${OSL_DLL} DESTINATION ${SORT_SOURCE_DIR}/bin/ )
endif( SORT_PLATFORM_WIN )
