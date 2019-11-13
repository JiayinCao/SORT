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

cmake_minimum_required (VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# define platform
if (WIN32)
    set(SORT_PLATFORM_WIN true)
    set(SORT_PLATFORM_MAC false)
    set(SORT_PLATFORM_LINUX false)
    set(SORT_PLATFORM_NAME "Windows" )
elseif(APPLE)
    set(SORT_PLATFORM_WIN false)
    set(SORT_PLATFORM_MAC true)
    set(SORT_PLATFORM_LINUX false)
    set(SORT_PLATFORM_NAME "Mac OS" )
elseif(UNIX)
    set(SORT_PLATFORM_WIN false)
    set(SORT_PLATFORM_MAC false)
    set(SORT_PLATFORM_LINUX true)
    set(SORT_PLATFORM_NAME "Linux" )
endif()

# Set it to on to enable more debugging output
#set(DEBUG_CMAKE ON)

SET( ENABLE_PROFILER               "NO"   CACHE BOOL "Enable SORT profiling system. It is disabled by default." )
SET( ENABLE_STATS                  "YES"  CACHE BOOL "Enable SORT stats system. It is enabled by default." )
SET( ENABLE_FASTMATH               "NO"   CACHE BOOL "Enable fast math. It may have potential risk in errors due to lower precision. Performance gain is quite limited and unstable, for which reason it is disabled by default." )
SET( ENABLE_LINKTIME_OPTIMIZATION  "YES"  CACHE BOOL "Link time optimization is enabled by default since it does show some performance gain sometimes." )

# For Easy_Profiler to locate its library, but this doesn't need to show up as UI an option
if(ENABLE_PROFILER)
    set( easy_profiler_DIR "./dependencies/easy_profiler/lib/cmake/easy_profiler" )
    MARK_AS_ADVANCED( easy_profiler_DIR )
endif(ENABLE_PROFILER)

# default build type is release
set( DEFUALT_BULID_TYPE "Release")

# Options to choose configuration type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFUALT_BULID_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFUALT_BULID_TYPE}" CACHE STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

project (SORT)

# include other modules
include("${CMAKE_CURRENT_LIST_DIR}/build-files/cmake/externalPackages.cmake")

if(ENABLE_PROFILER)
	find_package(easy_profiler REQUIRED)
endif(ENABLE_PROFILER)

include_directories( "${SORT_SOURCE_DIR}/src" )

macro(source_group_by_dir source_files)
    set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
    foreach(sgbd_file ${${source_files}})
        string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
        string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
        string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        if(sgbd_nogroup)
            set(sgbd_group_name "\\")
        endif(sgbd_nogroup)
        source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach(sgbd_file)
endmacro(source_group_by_dir)

file(GLOB_RECURSE project_headers src/*.h)
file(GLOB_RECURSE project_cpps src/*.cpp)
file(GLOB_RECURSE project_cs src/*.c)
file(GLOB_RECURSE project_ccs src/*.cc)

set(all_files ${project_headers} ${project_cpps} ${project_cs} ${project_ccs})
source_group_by_dir(all_files)

file(GLOB_RECURSE thirdparty_headers src/thirdparty/*.h)
file(GLOB_RECURSE thirdparty_cpps src/thirdparty/*.cpp)
file(GLOB_RECURSE thirdparty_cs src/thirdparty/*.c)
file(GLOB_RECURSE thirdparty_ccs src/thirdparty/*.cc)
set(thirdparty_files ${thirdparty_headers} ${thirdparty_cpps} ${thirdparty_cs} ${thirdparty_ccs})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SORT_SOURCE_DIR}/bin")

add_executable(SORT ${all_files})

target_link_libraries(SORT ${OPENIMAGEIO_LIBRARIES})
target_link_libraries(SORT ${OSL_LIBRARIES})
if(ENABLE_PROFILER)
    target_link_libraries(SORT easy_profiler)
endif(ENABLE_PROFILER)

# Enable multi-thread compiling on Windows.
if (SORT_PLATFORM_WIN)
    file(GLOB easy_profiler_bin "${SORT_SOURCE_DIR}/dependencies/easy_profiler/bin/*.dll")
    file(COPY ${easy_profiler_bin} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    include(ProcessorCount)
    ProcessorCount(N)
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP${N}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")
endif (SORT_PLATFORM_WIN)

# Enable SORT Stats system.
if(ENABLE_STATS)
    message( STATUS "SORT Stats System Enabled.")
	add_definitions(-DSORT_ENABLE_STATS_COLLECTION)
else()
    message( STATUS "SORT Stats Sysatem Disabled." )
endif(ENABLE_STATS)

# Enable Profiling system in SORT.
if(ENABLE_PROFILER)
    message( STATUS "SORT Profiling System Enabled." )
    add_definitions(-DENABLE_PROFILER)
if (!SORT_PLATFORM_LINUX) # it doesn't link well with OSL
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()
else()
    message( STATUS "SORT Profiling System Disabled." )
endif(ENABLE_PROFILER)

# Surpress a warning
if(SORT_PLATFORM_MAC)
    add_definitions( -Wno-deprecated-register )
endif(SORT_PLATFORM_MAC)

# Setup correct output name for different configurations
set_target_properties( SORT PROPERTIES RELEASE_OUTPUT_NAME "sort_r" )
set_target_properties( SORT PROPERTIES DEBUG_OUTPUT_NAME "sort_d" )
set_target_properties( SORT PROPERTIES RELWITHDEBINFO_OUTPUT_NAME "sort_rd" )

# Specific settings in windows
if(MSVC)
	# enable whole program optimization, which gains roughly 8% in term of performance
	if(ENABLE_LINKTIME_OPTIMIZATION)
		set_target_properties( SORT PROPERTIES COMPILE_FLAGS "/GL" )
		set_target_properties( SORT PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
	endif()

	# enable fast math for better performance
	if(ENABLE_FASTMATH)
		set_target_properties( SORT PROPERTIES COMPILE_FLAGS "/fp:fast" )
	endif()

	# this enables debuging in Visual Studio, otherwise it will crash
	# somehow CMAKE_MSVC_RUNTIME_LIBRARY doesn't work
	set_target_properties( SORT PROPERTIES COMPILE_FLAGS "/MD" )

	set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS /W0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4800" )
endif(MSVC)

# Specific settings in Linux and Mac
if( SORT_PLATFORM_MAC OR SORT_PLATFORM_LINUX )
	set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS -w)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")

	if(ENABLE_LINKTIME_OPTIMIZATION)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
	endif()

	if(ENABLE_FASTMATH)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
	endif()
endif()

# Output include directories
if( DEBUG_CMAKE )
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    message(STATUS "Including directories:")
    foreach(dir ${dirs})
      message(STATUS "\t'${dir}'")
    endforeach()
endif()

MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
