/*
   FileName:      integrator.h

   Created Time:  2013-06-26 12:27:05

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "integrator.h"
#include "system.h"

extern System g_System;

Integrator::Integrator()
	:scene( g_System.GetScene() )
{
	max_recursive_depth=6;
}