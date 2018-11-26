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

// include the header file
#include "orennayar.h"
#include "bsdf.h"

// constructor
OrenNayar::OrenNayar( const Spectrum& reflectance , float roughness, const Spectrum& weight , const Vector& n, bool doubleSided) : Bxdf( weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION) , n, doubleSided) , R(reflectance)
{
	// roughness ranges from 0 to infinity
	roughness = std::max( 0.0f , roughness );

	const float roughness2 = roughness * roughness;
	A = 1.0f - (roughness2 / ( 2.0f * (roughness2+0.33f)));
	B = 0.45f * roughness2 / ( roughness2 + 0.09f );
}

// evaluate bxdf
// para 'wo' : out going direction
// para 'wi' : in direction
// result    : the portion that comes along 'wo' from 'wi'
Spectrum OrenNayar::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

	float sintheta_i = SinTheta(wi);
	float sintheta_o = SinTheta(wo);
	
	float sinphii = SinPhi(wi);
	float cosphii = CosPhi(wi);
	float sinphio = SinPhi(wo);
	float cosphio = CosPhi(wo);
	float dcos = cosphii * cosphio + sinphii * sinphio;
	if( dcos < 0.0f ) dcos = 0.0f;
	
	float abs_cos_theta_o = (float)AbsCosTheta(wo);
	float abs_cos_theta_i = (float)AbsCosTheta(wi);
	
	if( abs_cos_theta_i < 0.00001f && abs_cos_theta_o < 0.00001f )
		return 0.0f;
	
	float sinalpha , tanbeta;
	if( abs_cos_theta_o > abs_cos_theta_i )
	{
		sinalpha = sintheta_i;
		tanbeta = sintheta_o / abs_cos_theta_o;
	}else
	{
		sinalpha = sintheta_o;
		tanbeta = sintheta_i / abs_cos_theta_i;
	}
	
	return R * INV_PI * ( A + B * dcos * sinalpha * tanbeta ) * AbsCosTheta(wi);
}
