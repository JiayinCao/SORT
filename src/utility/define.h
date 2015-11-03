/*
   FileName:      define.h

   Created Time:  2011-08-04 12:42:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_DEFINE
#define	SORT_DEFINE

#include "sort.h"

#define	PI		3.1415926f
#define	TWO_PI	6.2831852f
#define	INV_PI	0.3183099f
#define INV_TWOPI 0.15915494f

// some useful macro
#define SAFE_DELETE(p) { if(p) { delete p; p = 0; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] p; p = 0 ; } }

#if defined(SORT_IN_WINDOWS)
#define Thread_Local __declspec(thread)
#elif defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)
#define Thread_Local __thread
#endif

// math macros
#define saturate(x) max(0.0f,min(1.0f,x))
inline float clamp( float x , float mi , float ma )
{
	if( x > ma ) x = ma;
	if( x < mi ) x = mi;
	return x;
}

#endif
