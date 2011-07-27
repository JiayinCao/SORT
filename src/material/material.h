/*
 * filename :	material.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATERIAL
#define	SORT_MATERIAL

// include the cpp file
#include "utility/propertyset.h"
#include "utility/creator.h"

// pre-declera classes
class Bsdf;
class Intersection;

///////////////////////////////////////////////////////////
// definition of material
class Material : public PropertySet<Material>
{
// public method
public:
	// default constructor
	Material(){}
	// destructor
	virtual ~Material() {}

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const = 0;
};

#endif
