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

enum SE_Flag : unsigned int{
    SE_NONE             = 0x00,
    SE_EVALUATE_BXDF    = 0x01,                                         // Parse bxdf in the material
    SE_EVALUATE_BSSRDF  = 0x02,                                         // Parse bssrdf in the material
    SE_EVALUATE_ALL     = ( SE_EVALUATE_BXDF | SE_EVALUATE_BSSRDF ),    // Everything will be parsed in the material

    SE_REPLACE_BSSRDF   = 0x04,                                         // Whether to replace BSSRDFD with lambert
    SE_SUB_EVENT        = 0x08,                                         // Whether this scattering event is a sub-event of others
};

#define SE_MAX_BXDF_COUNT          16      // Maximum number of bxdf in a material is 16 by default
#define SE_MAX_BSSRDF_COUNT        4       // Maximum number of bssrdf in a material is 4 by default 

class Bxdf;

//! @brief  ScatteringEvent is a bsdf/bssrdf holder that could hold multiple of each.
/**
 * ScatteringEvent is the new 'BSDF' class, which not only holds BRDF/BTDF, but also holds BSSRDF for sub-surface 
 * scattering. This is more advanced than previous 'BSDF' implementation in a way that it supports multiple layer 
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
    //! @param flag                 Scattering event flag.              
    ScatteringEvent( const Intersection& intersection , const SE_Flag flag = SE_EVALUATE_ALL );

    //! @brief  Add a new bxdf in the scattering event, there will be at most 16 bxdfs in it.
    //!
    //! Adding more than 16 bxdf will result in being ignored by the system, which may lead to energy loss in the
    //! material.
    //! The scattering event is not responsible for maintaining the allocated memory passed in, neither is it
    //! responsible for keeping the memory alive during its life time. It is higher level code's job to keep the
    //! memory alive.
    //!
    //! @param  bxdf        The bxdf to be added.
    SORT_FORCEINLINE  void    AddBxdf( const Bxdf* bxdf ){
        if( m_bxdfCnt == SE_MAX_BXDF_COUNT || bxdf == nullptr || bxdf->GetEvalWeight().IsBlack() ) 
            return;
        m_bxdfs[m_bxdfCnt++] = bxdf;
        m_bxdfTotalSampleWeight += bxdf->GetSampleWeight();
    }

    //! @brief  Add a bssrdf in the scattering event, there will be at most 4 bssrdf in it.
    //!
    //! Adding more than 4 bssrdfs will result in being ignored by the system, which may lead to energy loss in the
    //! material.
    //! The scattering event is not responsible for maintaining the allocated memory passed in, neither is it
    //! responsible for keeping the memory alive during its life time. It is higher level code's job to keep the
    //! memory alive.
    //!
    //! @param  bssrdf      The bssrdf to be added.
    SORT_FORCEINLINE  void    AddBssrdf( const Bssrdf* bssrdf ){
        if( m_bssrdfCnt == SE_MAX_BSSRDF_COUNT || bssrdf == nullptr || bssrdf->GetEvalWeight().IsBlack() ) 
            return;
        m_bssrdfs[m_bssrdfCnt++] = bssrdf;
        m_bssrdfTotalSampleWeight += bssrdf->GetSampleWeight();
    }

    //! @brief Get intersection information of the point at which the bsdf is evaluated.
    //! @return The intersection information of the point at which the bsdf is evaluated.
    SORT_FORCEINLINE const Intersection& GetIntersection() const {
        return m_intersection;
    }

    //! @brief  Get the flag of this scattering event.
    //!
    //! @return  The current flag of this scattering event.
    SORT_FORCEINLINE SE_Flag    GetFlag() const {
        return m_flag;
    }

    //! @brief  Randomly pick between bxdf and bssrdf
    //!
    //! @param  flag    Which catagory it picks, it could be SE_EVALUATE_BXDF/SE_EVALUATE_BSSRDF.
    //! @return         The properbility of picking the bxdf/bssrdf.
    float       SampleScatteringType( SE_Flag& flag ) const;

    //! @brief Evaluate the value of BSDF based on the incident and outgoing directions.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated value of the BSDF.
    Spectrum    Evaluate_BSDF( const Vector& wo , const Vector& wi ) const;

    //! @brief Importance sampling for the bsdf.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum    Sample_BSDF( const Vector& wo , Vector& wi , const class BsdfSample& bs , float& pdf ) const;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param type The specific bxdf type it considers during evaluation.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float       Pdf_BSDF( const Vector& wo , const Vector& wi ) const;

    //! @brief  Importance sample the incident direction and position.
    //!
    //! @param  scene   The scene where ray tracing happens.
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  inter   Incident intersection sampled.
    //! @param  pdf     The pdf of sampling this bssrdf among all bssrdfs.
    void        Sample_BSSRDF( const Scene& scene , const Vector& wo , const Point& po , BSSRDFIntersections& inter , float& pdf ) const;

private:
    const Bxdf*         m_bxdfs[SE_MAX_BXDF_COUNT]      = { nullptr };     /**< All bsdfs in the scattering event. */
    unsigned            m_bxdfCnt                       = 0;               /**< Number of bxdfs in the scattering event. */
    float               m_bxdfTotalSampleWeight         = 0.0f;            /**< Total weight of BXDF. */
    const Bssrdf*       m_bssrdfs[SE_MAX_BSSRDF_COUNT]  = { nullptr };     /**< All bssrdfs in the scattering event. */
    unsigned            m_bssrdfCnt                     = 0;               /**< Number of bssrdfs in the scattering event. */
    float               m_bssrdfTotalSampleWeight       = 0.0f;            /**< Total weight of BSSRDF. */

    const SE_Flag       m_flag;             /**< Some scattering event is under other scattering event, like 'Blend' and 'Coat'. */
    Vector              m_n;                /**< Normal at the point to be evaluated. */
    Vector              m_t;                /**< Bi-tangent at the point to be evaluated. */
    Vector              m_bt;               /**< Tangent at the point to be evaluated. */
    const Intersection& m_intersection;     /**< Intersection at the point to be evaluated. */

    //! @brief  Transform a vector from world coordinate to local shading coordinate.
    //!
    //! @param v        A vector in world coordinate.
    //! @param force    Force transform regardless whether this scattering event is a sub-event or not.
    //! @return         The corresponding vector in local shading coordinate.
    SORT_FORCEINLINE Vector worldToLocal( const Vector& v , bool force = false ) const;

    //! @brief  Transform a vector from shading coordinate to world coordinate.
    //!
    //! @param v        A vector in shading coordinate.
    //! @return         The corresponding vector in world coordinate.
    SORT_FORCEINLINE Vector localToWorld( const Vector& v ) const;
};