/*
 * filename :	refraction.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "refraction.h"
#include "geometry/vector.h"

// to be deleted
#include "managers/logmanager.h"
	
// evaluate bxdf
Spectrum Refraction::f( const Vector& wo , const Vector& wi ) const
{
	return Spectrum();
}

// sample a direction randomly
Spectrum Refraction::Sample_f( const Vector& wo , Vector& wi , float* pdf ) const
{

	return Spectrum();
}
