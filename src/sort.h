/*
   FileName:      sort.h

   Created Time:  2011-08-04 12:41:38

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT__
#define SORT__

// include the header files
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>

// use default name space
using namespace std;

// find the correct platform
// currently , windows and linux is support
#if defined(_WIN32) || defined(_WIN64)
	#define SORT_IN_WINDOWS
#elif defined(__linux__)
	#define SORT_IN_LINUX
#endif

// enable debug by default
#define	SORT_DEBUG

#endif
