/*
 * filename :	sort.h
 *
 * programmer : Cao Jiayin
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

#endif
