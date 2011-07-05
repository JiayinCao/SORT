/*
 * filename :	material.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATERIAL
#define	SORT_MATERIAL

// pre-declera classes
class Bsdf;
class Intersection;

///////////////////////////////////////////////////////////
// definition of material
class Material
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
