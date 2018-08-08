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

// include header file
#include "bxdf.h"

//! @brief OrenNayar BRDF.
/**
 * 'Generalization of Lambert’s Reflectance Model'
 * http://www1.cs.columbia.edu/CAVE/publications/pdfs/Oren_SIGGRAPH94.pdf
 *
 * Like lambert BRDF, OrenNayar presents similar quality with subtle differences.
 * It has slightly brighter color at the contour.
 */
class OrenNayar : public Bxdf
{
public:
	//! Constructor
    //! @param reflectance  Direction-hemisphere reflection.
    //! @param roughness    It controls the roughness of the surface.
    //! @param weight       Weight of the BRDF
	OrenNayar( const Spectrum& reflectance , float roughness , const Spectrum& weight , const Vector& n, bool doubleSided = false);
	
    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
private:
	const Spectrum R;   /**< Direction-Hemisphere reflection or total reflection. */
	float	A;          /**< Internal data for OrenNayar computation. */
	float	B;          /**< Internal data for OrenNayar computation. */
};
