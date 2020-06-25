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

#include "bxdf.h"
#include "core/resource.h"
#include "scatteringevent/bsdf/bxdf_utils.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeFourier, "fourier")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeFourier, void*, measured_data)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeFourier, float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeFourier)

//! @brief FourierBxdfData.
/**
 * Same with MERL brdf, FourierBxdf is also a measured bxdf.
 * However, it is much compact comparing with MERL in term of memory usage.
 * There is also an importance sampling method provided in the bxdf type.
 */
class FourierBxdfData : public Resource {
public:
    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const;

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
    //! @return     The evaluted BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;

    //! @brief Evalute the pdf of an existance direction given the Incident direction.
    //!
    //! If one implements customized sample_f for the brdf, it needs to have corresponding version of
    //! this function, otherwise it is not unbiased.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const;

    //! Load brdf data from Fourier Bxdf file.
    //!
    //! @param filename     Name of Fourier Bxdf file.
    //! @return             Whether the resource is loaded
    bool    LoadResource(const std::string& filename) override;

private:
    // Bxdf Table
    struct FourierBxdfTable{
        float   eta = 1.0f;
        int     nMax = 0;
        int     nChannels = 1;
        int     nMu = 0;
        std::unique_ptr<float[]>   mu = nullptr;
        std::unique_ptr<int[]>     m = nullptr;
        std::unique_ptr<int[]>     aOffset = nullptr;
        std::unique_ptr<float[]>   a = nullptr;
        std::unique_ptr<float[]>   a0 = nullptr;
        std::unique_ptr<float[]>   cdf = nullptr;
        std::unique_ptr<float[]>   recip = nullptr;

        float* GetAk( int offsetI , int offsetO , int* mptr ) const{
            const int offset = offsetO * nMu + offsetI;
            *mptr = m[offset];
            return a.get() + aOffset[offset];
        }
    };

    FourierBxdfTable    bsdfTable;

    // Fourier interpolation
    float fourier( const float* ak , int m , double cosPhi ) const;
    // Importance sampling for fourier interpolation
    // Refer these two wiki pages for further detail:
    // Bisection method :   https://en.wikipedia.org/wiki/Bisection_method
    // Newton method :      https://en.wikipedia.org/wiki/Newton%27s_method
    float sampleFourier( const float* ak , const float* recip , int m , float u , float* pdf , float* phiptr ) const;

    // Get CatmullRomWeights
    bool getCatmullRomWeights( float x , int& offset , float* weights ) const;

    // Importance sampling for catmull rom
    float sampleCatmullRom2D( int size1 , int size2 , const float* nodes1 , const float* nodes2 , const float* values , const float* cdf ,
                             float alpha , float u , float* fval , float* pdf ) const;

    // helper function to find interval that wraps the target value
    template<typename Predicate>
    int findInterval( int cnt , const Predicate& pred ) const;

    // helper functio to blend coefficients for fourier
    int blendCoefficients( float* ak , int channel , int offsetI , int offsetO , float* weightsI , float* weightsO ) const;
};

//! @brief FourierBxdf.
/**
 * Same with MERL brdf, FourierBxdf is also a measured bxdf.
 * However, it is much compact comparing with MERL in term of memory usage.
 * There is also an importance sampling method provided in the bxdf type.
 */
class FourierBxdf : public Bxdf
{
public:
    //! Constructor taking spectrum information.
    //!
    //! @param params       Parameter set.
    //! @param weight       Weight of this BRDF
    FourierBxdf(const ClosureTypeFourier& params, const Spectrum& weight);

    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override{
        return m_data->f(wo,wi);
    }

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
    //! @return     The evaluted BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override{
        return m_data->sample_f( wo , wi , bs , pdf ) * absCosTheta(wi);
    }

    //! @brief Evalute the pdf of an existance direction given the Incident direction.
    //!
    //! If one implements customized sample_f for the brdf, it needs to have corresponding version of
    //! this function, otherwise it is not unbiased.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override{
        return m_data->pdf( wo , wi );
    }

private:
    const FourierBxdfData*    m_data;   /**< The actual data of Fourier brdf. */
};
