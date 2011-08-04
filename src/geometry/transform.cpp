/*
   FileName:      transform.cpp

   Created Time:  2011-08-04 12:50:58

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "transform.h"
 
// check whether the transform is identity
bool Transform::IdIdentity() const
{
	if( matrix.m[0] != 1.0f || matrix.m[1] != 0.0f || matrix.m[2] != 0.0f || matrix.m[3] != 0.0f )
		return false;
	if( matrix.m[4] != 0.0f || matrix.m[5] != 1.0f || matrix.m[6] != 0.0f || matrix.m[7] != 0.0f )
		return false;
	if( matrix.m[8] != 0.0f || matrix.m[9] != 0.0f || matrix.m[10] != 1.0f || matrix.m[11] != 0.0f )
		return false;
	if( matrix.m[12] != 0.0f || matrix.m[13] != 0.0f || matrix.m[14] != 0.0f || matrix.m[15] != 1.0f )
		return false;

	return true;
}

// whether there is scale factor in the matrix
bool Transform::HasScale() const
{
	return matrix.HasScale();
}