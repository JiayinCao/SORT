/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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

	// sampling a normal respect to the NDF
	virtual Vector sample_f( const BsdfSample& bs ) const = 0;
};

class Blinn : public MicroFacetDistribution
{
// public method
public:
	// constructor
	Blinn( float roughness );
	
	// probabilty of facet with specific normal (v)
	float D(float NoH) const;
	
	// sampling according to Blinn
	// PBRT's method is not used here.
	// Check "Microfacet Models for Refraction through Rough Surfaces" for detail.
	virtual Vector sample_f( const BsdfSample& bs ) const;

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

	// sampling according to Beckmann
	virtual Vector sample_f( const BsdfSample& bs ) const;

// private field
private:
	float m;
	float alpha;
};

class GGX : public MicroFacetDistribution
{
// public method
public:
	GGX( float roughness );

	// probabilty of facet with specific normal (v)
	float D(float NoH) const;

	// sampling according to GGX
	virtual Vector sample_f( const BsdfSample& bs ) const;

// private field
private:
	float m = 0.0f;
    float alpha = 0.0f;
};

/////////////////////////////////////////////////////////////////////
// visibility term

class VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH ) = 0;
};

class VisImplicit : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);
};

class VisNeumann : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);
};

class VisKelemen : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);
};

class VisSchlick : public VisTerm
{
public:
	VisSchlick( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);

private:
	float roughness;
};

class VisSmith : public VisTerm
{
public:
	VisSmith( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);

private:
	float roughness;
};

class VisSmithJointApprox : public VisTerm
{
public:
	VisSmithJointApprox( float rough ): roughness(rough) {}
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);

private:
	float roughness;
};

class VisCookTorrance : public VisTerm
{
public:
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH);
};

class Microfacet : public Bxdf
{
// private field
protected:
	// reflectance
	Spectrum R;
	// distribution of facets
	MicroFacetDistribution* distribution = nullptr;
	// fresnel for the surface
	Fresnel* fresnel = nullptr;
	// visiblity term
	VisTerm* visterm = nullptr;

	// get reflected ray
	inline Vector	_getReflected( Vector v , Vector n ) const;

	// get refracted ray
	inline Vector	_getRefracted( Vector v , Vector n , float in_eta , float ext_eta , bool& inner_reflection ) const;
};

/////////////////////////////////////////////////////////////////////
// microfacet reflection bxdf
class MicroFacetReflection : public Microfacet
{
// public method
public:
	// constructor
	MicroFacetReflection(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v );
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	Spectrum f( const Vector& wo , const Vector& wi ) const;
	
	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'bs'  : bsdf sample variable
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;

	// get the pdf of the sampled direction
	// para 'wo' : out going direction
	// para 'wi' : coming in direction from light
	// result    : the pdf for the sample
	virtual float Pdf( const Vector& wo , const Vector& wi ) const;
};

/////////////////////////////////////////////////////////////////////
// microfacet refraction bxdf
// Refer to "Microfacet Models for Refraction through Rough Surfaces" for further detail
class MicroFacetRefraction : public Microfacet
{
// public method
public:
	// constructor
	MicroFacetRefraction(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v , float ieta , float eeta );
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	Spectrum f( const Vector& wo , const Vector& wi ) const;
	
	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'bs'  : bsdf sample variable
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;

	// get the pdf of the sampled direction
	// para 'wo' : out going direction
	// para 'wi' : coming in direction from light
	// result    : the pdf for the sample
	virtual float Pdf( const Vector& wo , const Vector& wi ) const;

// private field
private:
	// index of refraction
	float	eta_in;
	float	eta_ext;
};

#endif
