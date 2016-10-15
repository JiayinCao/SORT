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

//! @brief Interface for fresnel.
class	Fresnel
{
public:
	//! @brief Evalute the Fresnel term.
    //! @param cosi     Cosine value between the incoming ray and the normal.
    //! @param coso     Cosine value between the outgoing ray and the normal.
    //! @return         Evaluted fresnel value.
	virtual Spectrum Evaluate( float cosi , float coso) const = 0;
};

//! @brief A hack that presents no fresnel.
class	FresnelNo : public Fresnel
{
public:
    //! @brief Evalute the Fresnel term.
    //! @param cosi     Cosine value between the incoming ray and the normal.
    //! @param coso     Cosine value between the outgoing ray and the normal.
    //! @return         Evaluted fresnel value.
    Spectrum Evaluate( float cosi , float coso ) const override{
		return 1.0f;
	}
};

//! @brief Fresnel for conductors.
class	FresnelConductor : public Fresnel
{
public:
	//! Constructor
    //! @param  e   Index of refraction. TBD
    //! @param  kk  Absorbtion coefficient.
	FresnelConductor( const Spectrum& e , const Spectrum& kk ):
		eta(e) , k(kk)
	{
	}

    //! @brief Evalute the Fresnel term.
    //! @param cosi     Cosine value between the incoming ray and the normal.
    //! @param coso     Cosine value between the outgoing ray and the normal.
    //! @return         Evaluted fresnel value.
	Spectrum Evaluate( float cosi , float coso ) const override
	{
		float abs_cos = (coso>0.0f)?coso:(-coso);
		float sq_cos = abs_cos * abs_cos;

		Spectrum t = 2 * eta * abs_cos;
		Spectrum tmp_f = eta*eta+k*k;
		Spectrum tmp = tmp_f * sq_cos;
		Spectrum Rparl2 = (tmp - t + 1 ) / ( tmp + t + 1 );
		Spectrum Rperp2 = (tmp_f - t + sq_cos)/(tmp_f + t + sq_cos );

		return (Rparl2+Rperp2)*0.5f;
	}

private:
    Spectrum eta;   /**< Internal data used for fresnel calculation. */
    Spectrum k;     /**< Internal data used for fresnel calculation. */
};

//! @brief Fresnel for dielectric.
class	FresnelDielectric : public Fresnel
{
public:
    //! Constructor
    //! @param  ei      Index of refraction of the medium on the side normal points. TBD
    //! @param  et      Index of refraction of the medium on the other side normal points. TBD
	FresnelDielectric( float ei , float et ):
        eta_t(et),eta_i(ei)
	{
	}

    //! @brief Evalute the Fresnel term.
    //! @param cosi     Cosine value between the incoming ray and the normal.
    //! @param coso     Cosine value between the outgoing ray and the normal.
    //! @return         Evaluted fresnel value.
	Spectrum Evaluate( float cosi , float coso ) const override
	{
		float cos_i = ( cosi > 0.0f )?cosi:(-cosi);
		float cos_o = ( coso > 0.0f )?coso:(-coso);
		float t0 = eta_t * cos_i;
		float t1 = eta_i * cos_o;
		float t2 = eta_i * cos_i;
		float t3 = eta_t * cos_o;

		float Rparl = ( t0 - t1 ) / ( t0 + t1 );
		float Rparp = ( t2 - t3 ) / ( t2 + t3 );

		return ( Rparl * Rparl + Rparp * Rparp ) * 0.5f;
	}

private:
    float eta_t;    /**< Internal data used for fresnel calculation. TBD */
    float eta_i;    /**< Internal data used for fresnel calculation. TBD */
};
