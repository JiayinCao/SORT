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

#include "spectrum/spectrum.h"
#include "math/vector3.h"
#include "scatteringevent/scatteringunit.h"

//! @brief BXDF type.
enum BXDF_TYPE{
    BXDF_NONE = 0,
    BXDF_DIFFUSE = 1,
    BXDF_GLOSSY = 2,
    BXDF_REFLECTION = 8,
    BXDF_TRANSMISSION = 16,
    BXDF_ALL_TYPES = BXDF_DIFFUSE | BXDF_GLOSSY ,
    BXDF_ALL_REFLECTION = BXDF_ALL_TYPES | BXDF_REFLECTION ,
    BXDF_ALL_TRANSMISSION = BXDF_ALL_TYPES | BXDF_TRANSMISSION ,
    BXDF_ALL = BXDF_ALL_REFLECTION | BXDF_ALL_TRANSMISSION
};

//! @brief BRDF or BTDF.
/**
 * Bxdf is either BRDF(bidirectional reflection density function) or BTDF( bidirectional translation density function).\n
 * This class serves as a basic interface for varies bxdf types, it will not have any instance of itself. \n
 * One of the important concept used here is local coordinate and shading coordinate. Local coordinate means a local
 * coordinate where the up axis ( Y axis ) is exactly the geometry normal. Shading coordinate is slightly different from
 * local coordinate in the fact that its up axis could differ from geometry normal, named as a shading normal, usually
 * taken from normal map.
 */
class   Bxdf : public ScatteringUnit {
public:
    //! @brief  Constructor.
    //!
    //! @param ew       Evaluation weight of the bxdf.
    //! @param type     Type of the bxdf, currently not wisely used.
    Bxdf(const Spectrum& w, BXDF_TYPE type, Vector n, bool doubleSided = false);

    //! @brief  Constructor.
    //!
    //! @param ew       Evaluation weight of the bxdf.
    //! @param sw       Sampling weight of the bxdf;
    //! @param type     Type of the bxdf, currently not wisely used.
    Bxdf(const Spectrum& w, const float sw, BXDF_TYPE type, Vector n, bool doubleSided = false);

    //! @brief  Virtual destructor.
    virtual ~Bxdf() = default;

    //! Evaluate the BXDF.
    //!
    //! IMPORTANT: Instead of returning exactly the value of BXDF, this function choose to return BXDF * cos(\theta),
    //! where \theta is the angle between incident direction, usually the lighting direction, and the shading normal, which
    //! could be normal mapped.\n
    //! This is for better precision for some BXDF.
    //!
    //! @param  wo      The exitant direction in local space.
    //! @param  wi      The incident direction in local space.
    //! @return         Evaluated BRDF by cos(\theta)
    virtual Spectrum F( const Vector& wo , const Vector& wi ) const{
        return f( bsdfToBxdf(wo) , bsdfToBxdf(wi) );
    }

    //! Importance sampling for the BXDF.
    //!
    //! Each of the BXDF should have its own importance sampling method so that it can converge quickly to the expected
    //! result. Though, some of the BRDF, like MERL, doesn't have its own importance sampling method. It will fall back to
    //! the default importance sampling method, which is usually a source of fireflies.
    //!
    //! @param  wo      The exitant direction in local space.
    //! @param  wi      The outputted importance sampled direction in local space.
    //! @param  bs      Random sampling data, currently not wisely used.
    //! @param  pdf     The returned PDF to sample the picked incident direction w.r.t solid angle. It could be 'nullptr', nothing will be set.
    //! @return         Evaluted BRDF by cos(\theta)
    virtual Spectrum Sample_F( const Vector& wo , Vector& wi , const class BsdfSample& bs , float* pdf ) const{
        Spectrum res = sample_f( bsdfToBxdf(wo) , wi , bs , pdf );
        wi = bxdfToBsdf(wi);
        return res;
    }

    //! PDF of sampling the incident vector w.r.t the solid angle.
    //!
    //! @param  wo      The exitant direction in local space.
    //! @param  wi      The incident direction in local space.
    //! @return         The PDF w.r.t the solid angle to pick the incident direction (@param wi).
    virtual float Pdf( const Vector& wo , const Vector& wi ) const{
        return pdf( bsdfToBxdf(wo) , bsdfToBxdf(wi) );
    }

    //! @brief  Check the type of the bxdf, it shouldn't be overridden by derived classes.
    //!
    //! @param type     The type to check.
    //! @return         If the bxdf belongs to the input type.
    virtual bool    MatchFlag( BXDF_TYPE type ) const final {
        return (type & m_type)==m_type;
    }

    //! @brief  Get the type of the bxdf
    //!
    //! @return The specific type of the bxdf.
    virtual BXDF_TYPE GetType() const final {
        return m_type;
    }

protected:
    //! @brief Evaluate the BRDF.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    virtual Spectrum f( const Vector& wo , const Vector& wi ) const = 0;

    //! @brief  Importance sampling for the bxdf.
    //!
    //! This method is not pure virtual and it has a default implementation, which sample out-going
    //! directions that have linear probability with the cosine value between the out-going ray and
    //! the normal.\n
    //! However, it is suggested that each bxdf has its own importance sampling method for optimal
    //! convergence rate.\n
    //! One also needs to implement the function Pdf to make it consistent.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    virtual Spectrum sample_f( const Vector& wo , Vector& wi , const class BsdfSample& bs , float* pdf ) const;

    //! @brief  Evaluate the pdf of an exit direction given the Incident direction.
    //!
    //! If one implements customized sample_f for the brdf, it needs to have corresponding version of
    //! this function, otherwise it is not unbiased.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    virtual float pdf( const Vector& wo , const Vector& wi ) const;

    //! @brief  Helper function to decide if a vector is pointing on the other side of the primitive.
    //!
    //! The sideness is decided by geometry normal instead of shading normal.
    //!
    //! @param v    A vector in shading coordinate to be evaluated.
    //! @return     Whether the vector is pointing up in local coordinate.
    bool    PointingUp( const Vector& v ) const;

    //! @brief  This is a helpf function transforming vector from local coordinate to shading coordinate.
    //!
    //! Bxdf is only responsible for transforming vector from world space to local coordinate without taking
    //! normal map into consideration. Another transformation needs to be applied if there is normal map.
    //! Local coordinate is exactly the same with shading coordinate if there is no normal map.
    //! This is the function to perform the transformation if needed.
    //! Technically speaking, there is space for optimization to avoid one extra transformation by combining the
    //! two transformation together, SORT doesn't choose to do it for simplicity.
    //!
    //! @param v    A vector in local coordinate.
    //! @return     The corresponding vector in shading coordinate.
    SORT_FORCEINLINE Vector bsdfToBxdf( const Vector& v ) const{
        if( !normal_map_applied ) return v;
        return Vector( Dot(v,tn) , Dot(v,nn) , Dot(v,btn) );
    }

    //! @brief  Transform a vector from local coordinate to shading coordinate.
    //!
    //! Just like the above function transforming vectors from local coordinate to shading coordinate, it is
    //! necessary to transform it back to local coordinate so that BSDF can further transform it back to world
    //! coordinate.
    //!
    //! @param v    A vector in shading coordinate.
    //! @return     The corresponding vector in local space.
    SORT_FORCEINLINE Vector bxdfToBsdf( const Vector& v ) const{
        if( !normal_map_applied ) return v;
        return Vector(  v.x * tn.x + v.y * nn.x + v.z * btn.x ,
                        v.x * tn.y + v.y * nn.y + v.z * btn.y ,
                        v.x * tn.z + v.y * nn.z + v.z * btn.z );
    }

    //! @brief  Whether the two vectors are on the same side of the geometry.
    //!
    //! The geometry concept defined here is decided by geometry normal instead of shading normal, which could be
    //! normal mapped. Although the vectors passed in are in shading coordinate.
    //!
    //! @param  wi      Incident vector in shading coordinate.
    //! @param  wo      Exitant vector in shading coordinate.
    //! @return         Whether the two vectors are on the same side of the surface.
    SORT_FORCEINLINE bool SameHemiSphere(const Vector& wi, const Vector& wo) const {
        return !(PointingUp(wi) ^ PointingUp(wo));
    }

    BXDF_TYPE   m_type = BXDF_NONE;         /**< The specific type of the bxdf. */
    bool        normal_map_applied = false; /**< Whether normal map is applied on the BXDF. */
    Vector      nn;                         /**< Normal at the point to be Evaluated. */
    Vector      tn;                         /**< Tangent at the point to be Evaluated. */
    Vector      btn;                        /**< Bi-tangent at the point to be evaluated. */
    mutable Vector      gnormal;            /**< Geometry normal. */
    const bool  doubleSided = false;        /**< Whether the bxdf is double sided. */
};
