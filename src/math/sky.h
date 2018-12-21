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

// include the header
#include "spectrum/spectrum.h"
#include "core/propertyset.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "texture/imagetexture.h"
#include "core/samplemethod.h"

/*
//////////////////////////////////////////////////////////////////
//	definition of sky
//	note: we could also use a very large box or sphere to replace
//		  the sky we need , while that would make some accelerator
//		  such as uniform grid inefficient. So we use a isolated
//		  sky to show the environment.
class	Sky : public PropertySet<Sky>
{
public:
	// default constructor
	Sky(){}
	// destructor
	virtual ~Sky(){}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Vector& r ) const = 0;

	// get the average radiance
	virtual Spectrum GetAverage() const = 0;

	// sample direction
	virtual Vector sample_v( float u , float v , float* pdf , float* area_pdf ) const = 0;

	// get the pdf
	virtual float Pdf( const Vector& wi ) const = 0;

	
};*/

////////////////////////////////////////////////////////////////////////
// definition of sky sphere
class	Sky
{
public:
    // destructor
    ~Sky() { SAFE_DELETE(distribution); }

    // evaluate value from sky
    // para 'r' : the ray which misses all of the triangle in the scene
    // result   : the spectrum in the sky
    Spectrum Evaluate(const Vector& r) const;

    // get the average radiance
    Spectrum GetAverage() const;

    // sample direction
    Vector sample_v(float u, float v, float* pdf, float* area_pdf) const;

    // get the pdf
    float Pdf(const Vector& wi) const;

    // load image file
    void Load(const std::string& str) {
        m_sky.LoadImageFromFile(str);
        _generateDistribution2D();
    }

    // setup transformation
    void SetTransform(const Transform& tf) {
        m_transform = tf;
    }

private:
    Transform m_transform;

    ImageTexture	m_sky;
    class Distribution2D*	distribution = nullptr;

    // generate 2d distribution
    void _generateDistribution2D();
};
