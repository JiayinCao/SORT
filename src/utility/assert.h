/*
   FileName:      assert.h

   Created Time:  2011-08-04 20:03:11

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_ASSERT
#define	SORT_ASSERT

#include "sort.h"
#include "managers/logmanager.h"

// assert
#ifndef SORT_DEBUG
	#define Sort_Assert(expr) (void(0))
#else
	#define Sort_Assert(expr) if((expr)==false) LOG_ERROR<<(#expr)<<CRASH
#endif

#endif