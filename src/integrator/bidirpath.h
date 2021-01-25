/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
#include "math/interaction.h"
#include "scatteringevent/scatteringevent.h"

class   Light;

struct BDPT_Vertex{
    Point               p;                  // the position of the vertex
    Vector              n;                  // the normal of the vertex
    Vector              wi;                 // in direction
    Vector              wo;                 // out direction
    ScatteringEvent*    se = nullptr;       // scattering event
    float               rr = 0.0f;          // russian roulette
    Spectrum            throughput;         // through put
    SurfaceInteraction  inter;              // intersection

    // For further detail, please refer to the paper "Implementing Vertex Connection and Merging"
    // MIS factors
    double      vc = 0.0f;
    double      vcm = 0.0f;

    // depth of the vertex
    int         depth = 0;
};

struct Pending_Sample{
    Vector2i    coord;
    Spectrum    radiance;
};

//! @brief  Bidirectional path tracing integrator.
/**
 * Different from path tracing algorithm, which could end up in having trouble finding paths with reasonable 
 * contribution, bi-directional path tracing shoots rays from both sides, camera and lights, and then it connects
 * all paths generated from both sides.
 * The fact that bi-directional path tracing generates rays from light, allows it to evaluate Monte Carlo estimation
 * way more efficient than a standard path tracing algorithm.
 *
 * However, due to my limited spare time, there is no SSS and volume support in it for now. This is also not very high
 * priority in my to-do list for now.
 */
class BidirPathTracing : public Integrator{
public:
    DEFINE_RTTI( BidirPathTracing , Integrator );

    //! @brief  Evaluate the radiance along a specific direction.
    //!
    //! @param  ray             The ray to be tested with.
    //! @param  ps              Pixel sample used to evaluate Monte Carlo method.
    //! @param  scene           The scene to be evaluated.
    //! @return                 The radiance along the opposite direction that the ray points to.
    Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene) const override;


    //! @brief  The samples generated in this interface is not well used in this integrator for now.
    void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num ) override;

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        Integrator::Serialize( stream );
        stream >> m_bMIS;
    }

    //! @brief  Whether the integrater need final update, light tracing and bdpt will need it
    bool NeedFinalUpdate() const override{
        return true;
    }

protected:
    bool    light_tracing_only = false;     // only do light tracing
    int     sample_per_pixel = 1;           // light sample per pixel

    // compute G term
    Spectrum    _Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const;

    // connect light sample
    Spectrum    _ConnectLight(const BDPT_Vertex& eye_vertex, const Light* light , const Scene& scene ) const;

    // connect camera point
    void        _ConnectCamera(const BDPT_Vertex& light_vertex , int len , const Light* light , const Scene& scene ) const;

    // connect vertices
    Spectrum    _ConnectVertices( const BDPT_Vertex& light_vertex , const BDPT_Vertex& eye_vertex , const Light* light , const Scene& scene ) const;

private:
    // use multiple importance sampling to sample direct illumination
    bool    m_bMIS = true;

    // mis factor
    SORT_FORCEINLINE double MIS(double t) const {
        return m_bMIS ? t * t : 1.0f;
    }
    SORT_FORCEINLINE float MIS(float t) const {
        return m_bMIS ? t * t : 1.0f;
    }

    SORT_STATS_ENABLE( "Bi-directional Path Tracing" )
};
