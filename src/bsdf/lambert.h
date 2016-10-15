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

#include "bxdf.h"

//! @brief Lambert brdf.
/**
 * Lambert is the simplest BRDF that exists in any renderers.
 * It reflects equal radiance along all exitance directions.
 * One can use Lambert to simulate Matte-like material.
 */
class Lambert : public Bxdf
{
public:
	//! Default constructor setting default type value
    Lambert(){ m_type=BXDF_DIFFUSE; }
	//! Constructor taking spectrum information.
    //! @param s Direction-Hemisphere reflection.
    Lambert( const Spectrum& s ):R(s){m_type=BXDF_DIFFUSE;}

    //! Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const;

	//! Reset the directional-hemisphere reflection.
    //! @param color    Direction-Hemisphere reflection.
	void SetColor( const Spectrum& color ) { R = color; }

private:
	Spectrum R;         /**< Direction-Hemisphere reflection or total reflection. */
};
