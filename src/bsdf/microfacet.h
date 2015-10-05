/*
   FileName:      microfacet.h

   Created Time:  2013-03-17 19:08:55

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MICROFACET
#define	SORT_MICROFACET

// include header file
#include "bxdf.h"
#include "fresnel.h"

/////////////////////////////////////////////////////////////////////
// normal distribution

class MicroFacetDistribution
{
// public method
public:
	// probabilty of facet with specific normal (v)
	virtual float D(float NoH) const = 0;
};

class Blinn : public MicroFacetDistribution
{
// public method
public:
	// constructor
	Blinn( float roughness );
	
	// probabilty of facet with specific normal (v)
	float D(float NoH) const;
	
// private field
private:
	// the exponent
	float exp;
};

class Beckmann : public MicroFacetDistribution
{
// public method
public:
	Beckmann( float roughness );

	// probabilty of facet with specific normal (v)
	float D(float NoH) const;

// private field
private:
	float m;
};

class GGX : public MicroFacetDistribution
{
// public method
public:
	GGX( float roughness );

	// probabilty of facet with specific normal (v)
	float D(float NoH) const;

// private field
private:
	float m;
};

/////////////////////////////////////////////////////////////////////
// visibility term

class VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH ) = 0;
};

class VisImplicit : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH );
};

class VisNeumann : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH );
};

class VisKelemen : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH );
};

class VisSchlick : public VisTerm
{
public:
	VisSchlick( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float VoH );

private:
	float roughness;
};

class VisSmith : public VisTerm
{
public:
	VisSmith( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float NoH );

private:
	float roughness;
};

class VisSmithJointApprox : public VisTerm
{
public:
	VisSmithJointApprox( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float NoH );

private:
	float roughness;
};

/////////////////////////////////////////////////////////////////////
// microfacet bxdf
class MicroFacet : public Bxdf
{
// public method
public:
	// constructor
	MicroFacet(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v );
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	Spectrum f( const Vector& wo , const Vector& wi ) const;
	
// private field
private:
	// reflectance
	Spectrum R;
	// distribution of facets
	MicroFacetDistribution* distribution;
	// fresnel for the surface
	Fresnel* fresnel;
	// visiblity term
	VisTerm* visterm;
};

#endif
