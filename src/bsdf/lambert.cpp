/*
 * filename :	lambert.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "lambert.h"
#include "utility/define.h"
#include "geometry/vector.h"

// evaluate bxdf
Spectrum Lambert::f( const Vector& wo , const Vector& wi ) const
{
	return R * INV_PI;
}