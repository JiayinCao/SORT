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

#include "spectrum/spectrum.h"
#include "utility/enum.h"
#include "math/vector3.h"

class BsdfSample;

//! @brief Brdf or btdf.
/**
 * Bxdf is either brdf(bidirectional reflection density function) or btdf(
 * bidirectional translation density function).\n
 * This class serves as a basic interface for varies bxdf types, it will
 * not have any instance of itself.
 */
class	Bxdf
{
public:
    //! Constructor
    //! @param w    Weight of the bxdf
    //! @param type Type of the bxdf
    Bxdf(const Spectrum& w, BXDF_TYPE type, Vector n, bool doubleSided = false);

    //! Virtual destructor.
    virtual ~Bxdf(){}

    //! Get weight of this BXDF
    const Spectrum& GetWeight() const { return m_weight; }
    
    virtual Spectrum F( const Vector& wo , const Vector& wi ) const{
        return f( bsdfToBxdf(wo) , bsdfToBxdf(wi) );
    }
    virtual Spectrum Sample_F( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const{
        Spectrum res = sample_f( bsdfToBxdf(wo) , wi , bs , pdf );
        wi = bxdfToBsdf(wi);
        return res;
    }
    virtual float Pdf( const Vector& wo , const Vector& wi ) const{
        return pdf( bsdfToBxdf(wo) , bsdfToBxdf(wi) );
    }
    
    //! @brief  Check the type of the bxdf, it shouldn't be overridden by derived classes.
    //! @param type     The type to check.
    //! @return         If the bxdf belongs to the input type.
    virtual bool    MatchFlag( BXDF_TYPE type ) const final
    {return (type & m_type)==m_type;}
    
    //! @brief  Get the type of the bxdf
    //! @return The specific type of the bxdf.
    virtual BXDF_TYPE GetType() const final { return m_type; }
    
    //! @brief Update Geometry Normal
    //! @param n    Geometry normal in bsdf space.
    inline void UpdateGNormal( const Vector& n ) const { gnormal = bsdfToBxdf(n); }
    
protected:
    //! @brief Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const = 0;
    
    //! @brief Importance sampling for the bxdf.
    //!
    //! This method is not pure virtual and it has a default
    //! implementation, which sample out-going directions that have linear probability with the
    //! cosine value between the out-going ray and the normal.\n
    //! However, it is suggested that each bxdf has its own importance sampling method for optimal
    //! convergence rate.\n
    //! One also needs to implement the function Pdf to make it consistent.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;
    
    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //!
    //! If one implements customized sample_f for the brdf, it needs to have corresponding version of
    //! this function, otherwise it is not unbiased.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
	virtual float pdf( const Vector& wo , const Vector& wi ) const;

    //! @brief  Helper function to decide if a vector is pointing on the other side of the primitive
    //! @param  Vector to be evaluated
    bool    PointingUp( const Vector& v ) const;
    
    //! @brief Transform a vector from world coordinate to shading coordinate.
    //! @param v    A vector in world coordinate.
    //! @return     corresponding vector in shading coordinate.
    inline Vector bsdfToBxdf( const Vector& v ) const{
        if( !normal_map_applied ) return v;
        return Vector( Dot(v,sn) , Dot(v,nn) , Dot(v,tn) );
    }
    
    //! @brief Transform a vector from shading coordinate to world coordinate.
    //! @param v    A vector in shading coordinate.
    //! @return     corresponding vector in world coordinate.
    inline Vector bxdfToBsdf( const Vector& v ) const{
        if( !normal_map_applied ) return v;
        return Vector(  v.x * sn.x + v.y * nn.x + v.z * tn.x ,
                        v.x * sn.y + v.y * nn.y + v.z * tn.y ,
                        v.x * sn.z + v.y * nn.z + v.z * tn.z );
    }

    inline bool SameHemiSphere(const Vector& wi, const Vector& wo) const {
        return !(PointingUp(wi) ^ PointingUp(wo));
    }

    Spectrum    m_weight = 1.0f;            /**< The weight for the bxdf, usually between 0 and 1. */
    BXDF_TYPE   m_type = BXDF_NONE;         /**< The specific type of the bxdf. */
    bool        normal_map_applied = false; /**< Whether normal map is applied on the BXDF. */
    Vector      nn;                         /**< Normal at the point to be Evaluated. */
    Vector      sn;                         /**< Bi-tangent at the point to be evaluated. */
    Vector      tn;                         /**< Tangent at the point to be Evaluated. */
    mutable Vector      gnormal;            /**< Geometry normal. */
    const bool  doubleSided = false;        /**< Whether the bxdf is double sided. */
};
