#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.
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

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# print cmake version
message(STATUS "CMake Version: " ${CMAKE_MAJOR_VERSION} "." ${CMAKE_MINOR_VERSION} "." ${CMAKE_PATCH_VERSION} )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# this will avoid generate ZERO_CHECK project
set(CMAKE_SUPPRESS_REGENERATION true)

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

    # by default, we only build for Apple Silicon.
    # this is only useful for CMake GUI app since the command line version will already have the value ready detected by python script
    if("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    endif()
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
SET( ENABLE_SIMD_4WAY_OPTIMIZATION "YES"  CACHE BOOL "Enable SSE/Neon optimization, this could boost the performance of ray tracing." )
SET( ENABLE_SIMD_8WAY_OPTIMIZATION "NO"   CACHE BOOL "Enable AVX optimization, this could boost the performance of ray tracing even more." )
SET( ENABLE_INTEL_EMBREE           "YES"   CACHE BOOL "Enable Intel Embree." )

# detect apple silicon
set(APPLE_SILICON false)
if(SORT_PLATFORM_MAC)
    if(${CMAKE_OSX_ARCHITECTURES} STREQUAL "arm64")
        set(APPLE_SILICON true)
    endif()

    if(APPLE_SILICON)
        message(STATUS "Building for Apple Silicon Mac.")
    else()
        message(STATUS "Building for Intel Mac.")
    endif(APPLE_SILICON)
endif(SORT_PLATFORM_MAC)

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

project (SORT 
         LANGUAGES CXX C ASM)

# loading tsl lirary, this is mandatory
set(TSL_DIR ${SORT_SOURCE_DIR}/dependencies/tsl/)
find_package(TSL REQUIRED CONFIG)
message(STATUS "Tiny Shading Langauge version: ${TSL_VERSION}.")

# find embree if needed
if(ENABLE_INTEL_EMBREE)
    set(embree_DIR ${SORT_SOURCE_DIR}/dependencies/embree_3_13_0/lib/cmake/embree-3.13.0)
    find_package(embree 3.0)

    message(STATUS "Embree enabled in SORT.")

    # copy the dll manually if it is on windows
    if(SORT_PLATFORM_WIN)
       file(COPY ${SORT_SOURCE_DIR}/dependencies/embree_3_13_0/bin/embree3.dll DESTINATION ${SORT_SOURCE_DIR}/bin)
    endif(SORT_PLATFORM_WIN)
endif(ENABLE_INTEL_EMBREE)

# loading marl library
set(MARL_INCLUDE_DIR ${SORT_SOURCE_DIR}/dependencies/marl/include)
set(MARL_LIBRARY_DIR ${SORT_SOURCE_DIR}/dependencies/marl/lib)
set(MARL_LIBS        marl)

if(ENABLE_PROFILER)
    find_package(easy_profiler REQUIRED)
endif(ENABLE_PROFILER)

include_directories( "${SORT_SOURCE_DIR}/src" )
include_directories( ${TSL_INCLUDE_DIR} ${MARL_INCLUDE_DIR})

link_directories( ${TSL_LIBRARY_DIR} ${MARL_LIBRARY_DIR} )

file(GLOB_RECURSE project_headers src/*.h src/*.hpp)
file(GLOB_RECURSE project_cpps src/*.cpp)
file(GLOB_RECURSE project_cs src/*.c)
file(GLOB_RECURSE project_ccs src/*.cc)
file(GLOB_RECURSE project_asm src/*.S)

# some files are generated by python script
set(generated_src_dir ${SORT_SOURCE_DIR}/generated_src)
set(generated_src ${generated_src_dir}/fabric_lut.h ${generated_src_dir}/multi_scattering_lut.h)

# make sure this folder is included so that other source files can find these generated file without worrying about where they are
include_directories( "${generated_src_dir}" )

set(all_files ${project_headers} ${project_cpps} ${project_cs} ${project_ccs})

# for now, only fiber implementation uses assembly language
if(APPLE OR UNIX)
    list(APPEND all_files ${project_asm})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${all_files})

# tsl include folder
file(GLOB_RECURSE tsl_headers ${TSL_INCLUDE_DIR}/*.h)
source_group("thirdparty\\tsl" FILES ${tsl_headers})

# marl include folder
file(GLOB_RECURSE marl_headers ${MARL_INCLUDE_DIR}/*.h)
source_group("thirdparty\\marl" FILES ${marl_headers})

file(GLOB_RECURSE thirdparty_headers src/thirdparty/*.h)
file(GLOB_RECURSE thirdparty_cpps src/thirdparty/*.cpp)
file(GLOB_RECURSE thirdparty_cs src/thirdparty/*.c)
file(GLOB_RECURSE thirdparty_ccs src/thirdparty/*.cc)
set(thirdparty_files ${thirdparty_headers} ${thirdparty_cpps} ${thirdparty_cs} ${thirdparty_ccs})

if(ENABLE_SIMD_4WAY_OPTIMIZATION)
    add_definitions( -DSIMD_4WAY_ENABLED )
endif()

if(ENABLE_SIMD_8WAY_OPTIMIZATION)
    add_definitions( -DSIMD_8WAY_ENABLED )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SORT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SORT_SOURCE_DIR}/bin")

# add the executable
add_executable(SORT ${all_files} ${generated_src} ${tsl_headers})

# generated source, this should be generated with 'make setup' command
source_group( "generated src" FILES ${generated_src} )

target_link_libraries(SORT ${TSL_LIBS} ${MARL_LIBS})
if(ENABLE_PROFILER)
    target_link_libraries(SORT easy_profiler)
endif(ENABLE_PROFILER)

if(ENABLE_INTEL_EMBREE)
    # a helper macro so that the code knows whether Embree is supported
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_EMBREE_ENABLED=1" )

    # link the embree library
    target_link_libraries(SORT embree)
endif(ENABLE_INTEL_EMBREE)

# g-test needs the macro to avoid a compiling error in C++ 17
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GTEST_HAS_TR1_TUPLE} -DGTEST_HAS_TR1_TUPLE=0" )

# Enable multi-thread compiling on Windows.
if (SORT_PLATFORM_WIN)
    file(GLOB easy_profiler_bin "${SORT_SOURCE_DIR}/dependencies/easy_profiler/bin/*.dll")
    file(COPY ${easy_profiler_bin} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    include(ProcessorCount)
    ProcessorCount(N)
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP${N}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")

    # make sure to copy the tsl dll to bin folder
    tsl_runtime_copy( "${SORT_SOURCE_DIR}/bin" )

    # link socket libraries
    target_link_libraries(SORT "wsock32.lib;ws2_32.lib")
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
    add_definitions(-DSORT_ENABLE_PROFILER)

    if (!SORT_PLATFORM_LINUX) # it doesn't link well with OSL
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()

else()
    message( STATUS "SORT Profiling System Disabled." )
endif(ENABLE_PROFILER)

# Print out some setup information
if(ENABLE_SIMD_4WAY_OPTIMIZATION)
    if(APPLE_SILICON)
        message( STATUS "Neon optimization enabled.")
    else()
        message( STATUS "SSE optimization enabled.")
    endif(APPLE_SILICON)
endif(ENABLE_SIMD_4WAY_OPTIMIZATION)

if(ENABLE_SIMD_8WAY_OPTIMIZATION)
    if(NOT APPLE_SILICON)
        message( STATUS "AVX optimization enabled.")
    endif(NOT APPLE_SILICON)
endif(ENABLE_SIMD_8WAY_OPTIMIZATION)

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
        # the following line doesn't work somehow
        #set_target_properties( SORT PROPERTIES COMPILE_FLAGS_RELEASE "${COMPILE_FLAGS} /GL")

        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
        set_target_properties( SORT PROPERTIES LINK_FLAGS_RELEASE "${LINK_FLAGS} /LTCG")
    endif()

    # enable fast math for better performance
    if(ENABLE_FASTMATH)
        set_target_properties( SORT PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /fp:fast" )
    endif()

    # this enables debuging in Visual Studio, otherwise it will crash
    # somehow CMAKE_MSVC_RUNTIME_LIBRARY doesn't work
    set_target_properties( SORT PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /MD /EHsc" )

    set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS /W0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4800 /wd4251" )

    if(ENABLE_AVX_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX" )
    endif()
endif(MSVC)

# Specific settings in Linux and Mac
if( SORT_PLATFORM_MAC OR SORT_PLATFORM_LINUX )
    set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS -w)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O3")

    # link time optimization will result in bad any_cast in TSL on Ubuntu, :(
    # I would choose to disable it for the clean interface in TSL.
    if(ENABLE_LINKTIME_OPTIMIZATION AND NOT SORT_PLATFORM_LINUX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
    endif()

    if(ENABLE_SIMD_4WAY_OPTIMIZATION AND NOT APPLE_SILICON)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    endif()

    if(ENABLE_SIMD_8WAY_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
    endif()

    if(ENABLE_FASTMATH)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
    endif()
endif()

set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -DSORT_DEBUG")
set( CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DSORT_RELEASE")
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DSORT_RELEASE")

# Output include directories
if( DEBUG_CMAKE )
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    message(STATUS "Including directories:")
    foreach(dir ${dirs})
      message(STATUS "\t'${dir}'")
    endforeach()
endif()

# hide the options that I never use
MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
if(SORT_PLATFORM_MAC)
    MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES)
    MARK_AS_ADVANCED(CMAKE_OSX_DEPLOYMENT_TARGET)
    MARK_AS_ADVANCED(CMAKE_OSX_SYSROOT)
    MARK_AS_ADVANCED(CMAKE_EXECUTABLE_FORMAT)
endif()
