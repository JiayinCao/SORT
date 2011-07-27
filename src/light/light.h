/*
 * filename :	light.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_LIGHT
#define	SORT_LIGHT

// include header
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "geometry/transform.h"
#include "utility/creator.h"

// pre-decleration
class Intersection;
class Vector;

////////////////////////////////////////////////////////////////////////
// definition of ligth
class	Light : public PropertySet<Light>
{
// public method
public:
	// default constructor
	Light(){}
	// destructor
	virtual ~Light(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wo'		: output vector
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const = 0;

	// set transformation
	virtual void	SetTransform( const Transform& transform )
	{m_transform = transform;}

// protected field
protected:
	// transformation of the light
	Transform	m_transform;
};

#endif
