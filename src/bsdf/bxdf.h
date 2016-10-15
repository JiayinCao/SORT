/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
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
    //! Virtual destructor.
    virtual ~Bxdf(){}
    
    //! @brief Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const = 0;
    
    //! @brief Importance sampling for the bxdf.
    //!
    //! This method is not pure virtual and it has a default
    //! implementation, which sample out-going directions that have linear probability with the
    //! consine value between the out-going ray and the normal.\n
    //! However, it is suggested that each bxdf has its own importance sampling method for optimal
    //! convergence rate.\n
    //! One also needs to implement the function Pdf to make it consistance.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The evaluted BRDF value.
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;
    
    //! @brief Evalute the pdf of an existance direction given the incoming direction.
    //!
    //! If one implements customized sample_f for the brdf, it needs to have cooresponding version of
    //! this function, otherwise it is not unbiased.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The probabilty of choosing the out-going direction based on the incoming direction.
	virtual float Pdf( const Vector& wo , const Vector& wi ) const;

	//! @brief  Check the type of the bxdf, it shouldn't be overriden by derived classes.
    //! @param type     The type to check.
    //! @return         If the bxdf belongs to the input type.
	virtual bool	MatchFlag( BXDF_TYPE type ) const final
	{return (type & m_type)==m_type;}

	//! @brief  Get the type of the bxdf
    //! @return The specific type of the bxdf.
	virtual BXDF_TYPE GetType() const final { return m_type; }

	Spectrum	m_weight;           /**< The weight for the bxdf, usually between 0 and 1. */

protected:
	BXDF_TYPE m_type = BXDF_NONE;   /**< The specific type of the bxdf. */
};
