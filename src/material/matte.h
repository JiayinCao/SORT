/*
 * filename :	matte.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATTE
#define	SORT_MATTE

// include header file
#include "material.h"
#include "texture/texture.h"

//////////////////////////////////////////////////////////////////////
// definition of matte material
class Matte : public Material
{
// public method
public:
	// default constructor
	Matte();
	// destructor
	~Matte();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// set scaled color
	void SetDiffuse( Texture* tex ) { m_d = tex; }

// private field
private:
	// the scaled color for the material
	Texture* m_d;
};

#endif