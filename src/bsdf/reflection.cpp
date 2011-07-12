/*
 * filename :	reflection.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "reflection.h"
#include "geometry/vector.h"

// to be deleted
#include "managers/logmanager.h"
		
// evaluate bxdf
Spectrum Reflection::f( const Vector& wo , const Vector& wi ) const
{
	return Spectrum();
}

// sample a direction randomly
Spectrum Reflection::Sample_f( const Vector& wo , Vector& wi , float* pdf ) const
{
	LOG_ERROR<<"Not implemented."<<CRASH;

	return Spectrum();
}