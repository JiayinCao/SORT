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

// include the header file
#include "integrator.h"
#include "geometry/intersection.h"

class Bsdf;

struct VirtualLightSource
{
	Intersection	intersect;
	Vector			wi;
	Spectrum		power;
	int				depth;
};

/////////////////////////////////////////////////////////////////////////////
// definition of Instant Radiosity
// note : Instant Radiosity is a subset of directional path tracing
//        It has two seperate passes. First pass generates virtual light
//        sources along the path tracing from light sources. Second pass
//        will use those virtual light source to evaluate indirect
//        illumination. Direct illumination is handled the same way in
//        directlight integrator.
class	InstantRadiosity : public Integrator
{
public:

	DEFINE_CREATOR( InstantRadiosity , Integrator , "ir" );

	InstantRadiosity() {
		m_nLightPaths = 64;
		m_fMinDist = 1.0f;
		m_fMinSqrDist = 1.0f;
		m_nLightPathSet = 1;

		_registerAllProperty();
	}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;
    
	// Preprocess: In preprocessing stage, numbers of virtual light sources
    // are generated along the path tracing from light sources.
    virtual void PreProcess();
	// post-process after rendering
	virtual void PostProcess();

private:
	// light path set
	int		m_nLightPathSet;

    // number of paths to follow when creating virtual light sources
	int		m_nLightPaths;

	// distant threshold
	float	m_fMinDist;
	float	m_fMinSqrDist;

	// container for light sources
	std::list<VirtualLightSource>*	m_pVirtualLightSources;

	// register property
	void _registerAllProperty();

	// private method of li
	Spectrum _li( const Ray& ray , bool ignoreLe = false , float* first_intersect_dist = 0 ) const;

	class LightPathNumProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(LightPathNumProperty,Integrator);
		void SetValue( const std::string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
				ir->m_nLightPaths = atoi( str.c_str() );
		}
	};
	class MinDistanceProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(MinDistanceProperty,Integrator);
		void SetValue( const std::string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
			{
				ir->m_fMinDist = (float)atof( str.c_str() );
				ir->m_fMinSqrDist = ir->m_fMinDist * ir->m_fMinDist;
			}
		}
	};
	class LightPathSetProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(LightPathSetProperty,Integrator);
		void SetValue( const std::string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
				ir->m_nLightPathSet = (int)atof( str.c_str() );
		}
	};
    
    SORT_STATS_ENABLE( "Instant Radiosity" )
};
