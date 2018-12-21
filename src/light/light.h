/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// WARNING:
// Code in this folder is VERY VERY out-dated and badly implemented.
// I need to refactor this part of the renderer once I have time.

#include "spectrum/spectrum.h"
#include "math/transform.h"
#include "core/scene.h"
#include "math/vector3.h"

class Intersection;
class LightSample;
class Shape;

class Visibility
{
public:
	// default constructor
	Visibility( const Scene& s ):scene(s){}

	// whether it's visible from the light source
	bool	IsVisible() const{
		return !scene.GetIntersect( ray , 0 );
	}

	// the shadow ray
	Ray	ray;
	// the scene
	const Scene& scene;
};

////////////////////////////////////////////////////////////////////////
// definition of light
class	Light
{
public:
	// setup scene
	void	SetupScene( const Scene* s ) {scene=s;}
	
	// set transformation
	virtual void	SetTransform( const Transform& transform ) {light2world = transform;}

	// total power of the light
	virtual Spectrum Power() const = 0;

	// note : the following methods must be overwritten in non-delta light
	// whether the light is a delta light
	virtual bool IsDelta() const { return true; }

	// whether the light is an infinite light
	virtual bool IsInfinite() const { return false; }

	// get the shape of light
	virtual std::shared_ptr<Shape> GetShape() const { return nullptr; }

	// the pdf for specific sampled direction
	virtual float Pdf( const Point& p , const Vector& wi ) const { return 1.0f; }

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: probability density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const = 0;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the probability density function
	virtual Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const = 0;

	// sample light density
	virtual Spectrum Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const { return 0.0f; }

	// get intersection between the light and the ray
	virtual bool Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const { return false; }

	// set pdf of picking the light
	void SetPickPDF( float pdf ) {
		pickProp = pdf;
	}
	float PickPDF() const {
		return pickProp;
	}

protected:
	// scene containing the light
    const Scene* scene = nullptr;
	// intensity for the light
	Spectrum	intensity;
	// transformation of the light
	Transform	light2world;

	// pdf of picking the light
	float		pickProp;
};
