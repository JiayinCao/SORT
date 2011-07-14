/*
 * filename :	mirror.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MIRROR
#define	SORT_MIRROR

#include "material.h"

///////////////////////////////////////////////////////////////////////
// definition of mirror
class	Mirror : public Material
{
// public method
public:
	// default constructor
	Mirror();
	// destructor
	~Mirror();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// create instance of the brdf
	CREATE_INSTANCE( Mirror );

// private field
private:

	// register property
	void _registerAllProperty();
};

#endif
