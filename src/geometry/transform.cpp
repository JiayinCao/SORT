/*
 * filename :	transform.cpp
 *
 * programmer :	Cao Jiayin
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
	float l0 = Vector( matrix.m[0] , matrix.m[4] , matrix.m[8] ).Length();
	float l1 = Vector( matrix.m[1] , matrix.m[5] , matrix.m[9] ).Length();
	float l2 = Vector( matrix.m[2] , matrix.m[6] , matrix.m[7] ).Length();

#define	IS_ONE(x) ((x)>0.999f && (x)<1.001f )
	return !( IS_ONE(l0) && IS_ONE(l1) && IS_ONE(l2) );
#undef IS_ONE
}