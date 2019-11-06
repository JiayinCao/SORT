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

include (FindPackageHandleStandardArgs)
include (SelectLibraryConfigurations)

# Make sure for CMake to complain if anything goes wrong.
if (NOT DEBUG_CMAKE)
  set( OpenSL_FIND_QUIETLY      TRUE )
  set( OpenImageIO_FIND_QUIETLY TRUE )
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/findOpenImageIO.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/findOpenSL.cmake")
