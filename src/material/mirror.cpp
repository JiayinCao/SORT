/*
 * filename :	mirror.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "mirror.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/reflection.h"

// default constructor
Mirror::Mirror()
{
}

// destructor
Mirror::~Mirror()
{
}

// get bsdf
Bsdf* Mirror::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)(intersect);
	bsdf->AddBxdf( SORT_MALLOC(Reflection)() );

	return bsdf;
}