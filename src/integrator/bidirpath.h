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

#include "integrator.h"
#include "math/point.h"
#include "math/vector3.h"
#include "math/intersection.h"

class	Primitive;
class	Bsdf;
class   Light;

struct BDPT_Vertex
{
	Point			p;                  // the position of the vertex
	Vector			n;                  // the normal of the vertex
	Vector			wi;                 // in direction
	Vector			wo;                 // out direction
	Bsdf*			bsdf = nullptr;		// bsdf from material
	float			rr = 0.0f;          // russian roulette
	Spectrum		throughput;         // through put
	Intersection	inter;              // intersection

	// For further detail, please refer to the paper "Implementing Vertex Connection and Merging"
	// MIS factors
	double		vc = 0.0f;
	double      vcm = 0.0f;

	// depth of the vertex
    int			depth = 0;
};

struct Pending_Sample
{
	Vector2i	coord;
	Spectrum	radiance;
};

///////////////////////////////////////////////////////////////////////////////////
// definition of bidirectional path tracing
// BDPT is not finished yet, there are bugs in the following code.
// i'll try to finish it after i finish some more integrators.
class BidirPathTracing : public Integrator
{
public:
	DEFINE_CREATOR( BidirPathTracing , Integrator , "bdpt" );

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	Spectrum	Li( const Ray& ray , const PixelSample& ps ) const override;
    
    // request sample
    void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num ) override;

	// support pending write
	bool SupportPendingWrite() override { return true; }

	//! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
		Integrator::Serialize( stream );
		stream >> m_bMIS;
	}

    //! @brief      Serializing data to stream
    //!
    //! @param      Stream where the serialization data goes to. Depending on different situation, it could come from different places.#pragma endregion
    void    Serialize( OStreamBase& stream ) override {
		Integrator::Serialize( stream );
		stream << m_bMIS;
	}

protected:
	bool	light_tracing_only = false;		// only do light tracing
	int		sample_per_pixel = 1;           // light sample per pixel

	// compute G term
	Spectrum	_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const;

	// connect light sample
	Spectrum _ConnectLight(const BDPT_Vertex& eye_vertex, const std::shared_ptr<Light> light ) const;
	
	// connect camera point
	void _ConnectCamera(const BDPT_Vertex& light_vertex , int len , const std::shared_ptr<Light> light ) const;

	// connnect vertices
	Spectrum _ConnectVertices( const BDPT_Vertex& light_vertex , const BDPT_Vertex& eye_vertex , const std::shared_ptr<Light> light ) const;

private:
	// use multiple importance sampling to sample direct illumination
	bool	m_bMIS = true;

	// mis factor
    inline double MIS(double t) const {
        return m_bMIS ? t * t : 1.0f;
    }
    inline float MIS(float t) const {
        return m_bMIS ? t * t : 1.0f;
    }
    
    SORT_STATS_ENABLE( "Bi-directional Path Tracing" )
};
