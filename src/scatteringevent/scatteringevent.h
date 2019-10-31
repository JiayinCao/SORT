/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

#include "core/define.h"
#include "math/intersection.h"
#include "bssrdf/bssrdf.h"

#define SE_MAX_BXDF_COUNT          16      // Maximum number of bxdf in a material is 16 by default
#define SE_MAX_BSSRDF_COUNT        4       // Maximum number of bssrdf in a mterial is 4 by default 

class Bxdf;

//! @brief  This data structure holds the result of imporance sampling a bxdf.
struct SR_BxdfResult{
    Spectrum    m_val;    /**< The evaluated specturm of the incident and exitant direction with all bxdf in the scattering event. */
    Vector      m_wi;     /**< The incident direction sampled. */
    float       m_pdf;    /**< The pdf of sampling the incident direction. */
};

//! @brief  This data structure holds the result of imporance sampling a bssrdf.
struct SR_BssrdfResult{
    BSSRDFIntersections m_intersections;    /**< BSSRDF intersection. */
    float               m_pdf;              /**< The pdf of sampling this brssdf. */
};

//! @brief  This data structure holds the result of imporance sampling a scattering event.
struct ScatteringResult{
    bool    m_isSamplingBSSRDF = false;

    union{
        SR_BssrdfResult m_bssrdfResult;
        SR_BxdfResult   m_bxdfResult;
    };
};

//! @brief  ScatteringEvent is a bsdf/bssrdf holder that could hold multiple of each.
/**
 * ScatteringEvent is the new 'BSDF' class, which not only holds BRDF/BTDF, but also holds BSSRDF for sub-surface 
 * scattering. This is more advanced than previous 'BSDF' implementation in a way that it supports muliple layer 
 * of BSSRDFs and it handles BSSRDF the same way with BXDF.
 * The rest of the system will have the knowledge about which sample it takes, but the detail how it is picked and 
 * its pdf of picking it could be simply ignored.
 * This should be a fairly light layer with minimal performance overhead that hides some detail of picking 
 * bxdf/bssrdfs. Most data should be transparent to the rest of the system.
 */
class ScatteringEvent{
public:
    //! @brief Constructor taking intersection data.
    //!
    //! @param intersection         Intersection data of the point to be Evaluated.
    //! @param subScatteringEvent   Whether the scattering event is a sub-event belongs to ohter event.
    //!                             Ths happens in some cases like 'Coat' material can coat on top of another
    //!                             scattering event.
    ScatteringEvent( const Intersection& intersection , bool subScatteringEvent = false );

    //! @brief  Add a new bxdf in the scattering event, there will be at most 16 bxdfs in it.
    //!
    //! Adding more than 16 bxdf will result in being ignored by the system, which may lead to energy loss in the
    //! material.
    //! The scattering event is not responsible for maintaining the allcoated memory passed in, neither is it
    //! responsible for keeping the memory alive during its life time. It is higher level code's job to keep the
    //! memory alive.
    //!
    //! @param  bxdf        The bxdf to be added.
    inline  void    AddBxdf( const Bxdf* bxdf );

    //! @brief  Add a bssrdf in the scattering event, there will be at most 4 bssrdf in it.
    //!
    //! Adding more than 4 bssrdfs will result in being ignored by the system, which may lead to energy loss in the
    //! material.
    //! The scattering event is not responsible for maintaining the allcoated memory passed in, neither is it
    //! responsible for keeping the memory alive during its life time. It is higher level code's job to keep the
    //! memory alive.
    //!
    //! @param  bssrdf      The bssrdf to be added.
    inline  void    AddBssrdf( const Bssrdf* bssrdf );

    //! @brief  Randomly sample the scattering event by picking a BXDF/BSSRDF to evaluate.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    inline void     Sample_SE( const Vector& wo , const class BsdfSample& bs , ScatteringResult& sr ) const;

    //! @brief Get intersection information of the point at which the bsdf is evaluated.
    //! @return The intersection information of the point at which the bsdf is evaluated.
    inline const Intersection& GetIntersection() const {
        return m_intersection;
    }

private:
    const Bxdf*         m_bxdfs[SE_MAX_BXDF_COUNT]      = { nullptr };     /**< All bsdfs in the scattering event. */
    int                 m_bxdfCnt                       = 0;               /**< Number of bxdfs in the scattering event. */
    const Bssrdf*       m_bssrdfs[SE_MAX_BSSRDF_COUNT]  = { nullptr };     /**< All bssrdfs in the scattering event. */
    int                 m_bssrdfCnt                     = 0;               /**< Number of bssrdfs in the scattering event. */

    const bool          m_subEvent;         /**< Some scattering event is under other scattering event, like 'Blend' and 'Coat'. */
    Vector              m_n;                /**< Normal at the point to be evaluated. */
    Vector              m_t;                /**< Bi-tangent at the point to be evaluated. */
    Vector              m_bt;               /**< Tangent at the point to be evaluated. */
    const Intersection& m_intersection;     /**< Intersection at the point to be evaluated. */

    //! @brief  Transform a vector from world coordinate to local shading coordinate.
    //!
    //! @param v        A vector in world coordinate.
    //! @param force    Force transform regardless whether this scattering event is a sub-event or not.
    //! @return         The corresponding vector in local shading coordinate.
    inline Vector worldToLocal( const Vector& v , bool force = false ) const;

    //! @brief  Transform a vector from shading coordinate to world coordinate.
    //!
    //! @param v        A vector in shading coordinate.
    //! @return         The corresponding vector in world coordinate.
    inline Vector localToWorld( const Vector& v ) const;
};