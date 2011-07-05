/*
 * filename :	path.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PATH
#define	SORT_PATH

// include header file
#include "sort.h"

// get current directory
string GetExecutableDir();

// get full path
string GetFullPath( const string& str );

// set resource path
void SetResourcePath( const string& str );

// get current resource path
string GetResourcePath() ;

#endif
