/*
 * filename :	material.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATERIAL
#define	SORT_MATERIAL

// include the cpp file
#include "utility/propertyset.h"

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

	// create instance of the brdf
	virtual Material* CreateInstance() = 0;
};

#define	CREATE_INSTANCE(T)	Material* CreateInstance() { return new T(); }
#endif
