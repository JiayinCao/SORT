/*
   FileName:      path.h

   Created Time:  2011-08-04 12:43:08

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
