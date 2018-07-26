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

#include "bxdf.h"

//! @brief  MERL brdf.
/**
 * MERL is short for Mitsubishi Electric Research Laboratories. They provide some measured
 * brdf on the website http://www.merl.com/brdf/. Merl class is responsible for loading 
 * and displaying the brdf they provided in the renderer.\n
 * There is no importance sampling for this brdf. With the default sampling method, the
 * convergence rage is extremely low. 
 * The paper <a href="http://csbio.unc.edu/mcmillan/pubs/sig03_matusik.pdf">
 * "A Data-Driven Reflectance Model"</a> didn't propose an importance sampling method
 * for it. Further research is needed before it can be practical to be used.
 */
class Merl : public Bxdf
{
public:
	//! Default constructor setting brdf type.
    Merl(const Spectrum& weight = Spectrum(1.0f)) : Bxdf(weight, BXDF_ALL) {}

	//! Destructor deletes all allocated memory
    ~Merl() override;

    //! Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
    
	//! Load brdf data from MERL file.
    //! @param filename Name of the MERL file.
	void	LoadData( const string& filename );

	//! Whether there is valid data loaded.
    //! @return True if data is valid, otherwise it will return false.
	bool	IsValid() { return m_data != 0; }

private:
	double*	m_data = nullptr;   /**< The actual data of MERL brdf. */
};
