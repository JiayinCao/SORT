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

// include header file
#include "bxdf.h"

//! @brief OrenNayar brdf.
/**
 * Like lambert brdf, OrenNayar delievers similar quality with subtle differences.
 * It has slightly brighter color at the contour.
 */
class OrenNayar : public Bxdf
{
public:
	//! Contstructor
    //! @param reflectance  Direction-hemisphere reflection.
    //! @param roughness    It controls the roughness of the surface.
	OrenNayar( const Spectrum& reflectance , float roughness);
	
    //! Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
private:
	Spectrum R;         /**< Direction-Hemisphere reflection or total reflection. */
	float	A;          /**< Internal data for OrenNayar computation. */
	float	B;          /**< Internal data for OrenNayar computation. */
};
