cmake_minimum_required (VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET( easy_profiler_DIR "./dependencies/easy_profiler/lib/cmake/easy_profiler" CACHE PATH "Path to easy profiler" )

# Disable Profiling system by default
SET( SORT_ENABLE_PROFILER "NO" CACHE BOOL "Enable SORT profiling system. It is false by default." )
SET( SORT_ENABLE_STATS "YES" CACHE BOOL "Enable SORT stats system. It is true by default." )

# default build type is release
set( DEFUALT_BULID_TYPE "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFUALT_BULID_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFUALT_BULID_TYPE}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release")
endif()

project (SORT)

find_package(easy_profiler REQUIRED)

include_directories( "${SORT_SOURCE_DIR}/src" )

# dependencies include
include_directories( "${SORT_SOURCE_DIR}/dependencies/thirdparty/easy_profiler/include" )

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

add_executable(SORT ${all_files})

target_link_libraries(SORT easy_profiler)

if (WIN32)
file(GLOB easy_profiler_bin "${SORT_SOURCE_DIR}/dependencies/easy_profiler/bin/*.dll"
 							"${SORT_SOURCE_DIR}/dependencies/easy_profiler/bin/*.exe")
file(COPY ${easy_profiler_bin} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include(ProcessorCount)
ProcessorCount(N)
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP${N}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")

endif (WIN32)

if(SORT_ENABLE_STATS)
	add_definitions(-DSORT_ENABLE_STATS_COLLECTION)
endif(SORT_ENABLE_STATS)

if(SORT_ENABLE_PROFILER)
 	add_definitions(-DSORT_ENABLE_PROFILER)
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif(SORT_ENABLE_PROFILER)

if(UNIX)
	set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS -w)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")
endif(UNIX)

set_target_properties( SORT PROPERTIES RELEASE_OUTPUT_NAME "sort_r" )
set_target_properties( SORT PROPERTIES DEBUG_OUTPUT_NAME "sort_d" )

if(MSVC)
	set_source_files_properties(${thirdparty_files} PROPERTIES COMPILE_FLAGS /W0)
endif(MSVC)
