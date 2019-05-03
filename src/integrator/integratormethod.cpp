/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "integratormethod.h"
#include "math/ray.h"
#include "math/intersection.h"
#include "bsdf/bsdf.h"
#include "core/primitive.h"
#include "material/material.h"
#include "light/light.h"

inline float MisFactor( float f, float g ){
    return (f*f) / (f*f + g*g);
}

Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Light* light , const Intersection& ip , 
							const LightSample& ls ,const BsdfSample& bs , BXDF_TYPE type ){
	Bsdf* 	bsdf = nullptr;
	Bssrdf*	bssrdf = nullptr;	// not implemented yet
	ip.primitive->GetMaterial()->UpdateScattering( ip , bsdf , bssrdf );

	Spectrum radiance;
	Visibility visibility(scene);
	float light_pdf;
	float bsdf_pdf;
	const auto wo = -r.m_Dir;
	Vector wi;
	const auto li = light->sample_l( ip , &ls , wi , 0 , &light_pdf , 0 , 0 , visibility );
	if( light_pdf > 0.0f && !li.IsBlack() ){
		Spectrum f = bsdf->f( wo , wi , type );
		if( !f.IsBlack() && visibility.IsVisible() ){
			if( light->IsDelta() ){
				radiance += li * f / light_pdf;
			}else{
				bsdf_pdf = bsdf->Pdf( wo , wi , type );
				const auto weight = MisFactor( light_pdf , bsdf_pdf );
				radiance = li * f * weight / light_pdf;
			}
		}
	}

	if( !light->IsDelta() ){
		BXDF_TYPE bxdf_type;
		const auto f = bsdf->sample_f( wo , wi , bs , &bsdf_pdf , type , &bxdf_type );
		if( !f.IsBlack() && bsdf_pdf != 0.0f ){
			auto weight = 1.0f;
			// There is no delta BRDF in SORT.
			if( true ){
				float light_pdf;
				light_pdf = light->Pdf( ip.intersect , wi );
				if( light_pdf <= 0.0f )
					return radiance;
				weight = MisFactor( bsdf_pdf , light_pdf );
			}
			
			Spectrum li;
			Intersection _ip;
			if( false == light->Le( Ray( ip.intersect , wi ) , &_ip , li ) )
				return radiance;

			visibility.ray = Ray( ip.intersect , wi , 0 , 0.001f , _ip.t - 0.001f );
			if( !li.IsBlack() && visibility.IsVisible() )
				radiance += li * f * weight / bsdf_pdf;
		}
	}

	return radiance;
}