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

// include header files
#include "integratormethod.h"
#include "geometry/ray.h"
#include "geometry/intersection.h"
#include "bsdf/bsdf.h"
#include "geometry/primitive.h"
#include "material/material.h"
#include "light/light.h"

// evaluate direct lighting
Spectrum	EvaluateDirect( const Ray& r , const Scene& scene , const Light* light , const Intersection& ip , 
							const LightSample& ls ,const BsdfSample& bs , BXDF_TYPE type )
{
	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	Spectrum radiance;
	Visibility visibility(scene);
	float light_pdf;
	float bsdf_pdf;
	Vector wo = -r.m_Dir;
	Vector wi;
	Spectrum li = light->sample_l( ip , &ls , wi , 0 , &light_pdf , 0 , 0 , visibility );
	if( light_pdf > 0.0f && !li.IsBlack() )
	{
		Spectrum f = bsdf->f( wo , wi , type );
		if( f.IsBlack() == false && visibility.IsVisible() )
		{
			if( light->IsDelta() )
				radiance = li * f / light_pdf;
			else
			{
				bsdf_pdf = bsdf->Pdf( wo , wi , type );
				float power_hueristic = MisFactor( 1 , light_pdf , 1 , bsdf_pdf );
				radiance = li * f * power_hueristic / light_pdf;
			}
		}
	}

	if( !light->IsDelta() )
	{
		BXDF_TYPE bxdf_type;
		Spectrum f = bsdf->sample_f( wo , wi , bs , &bsdf_pdf , type , &bxdf_type );
		if( !f.IsBlack() && bsdf_pdf != 0.0f )
		{
			float weight = 1.0f;
			if( bxdf_type )
			{
				float light_pdf;
				light_pdf = light->Pdf( ip.intersect , wi );
				if( light_pdf <= 0.0f )
					return radiance;
				weight = MisFactor( 1 , bsdf_pdf , 1 , light_pdf );
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

// mutilpe importance sampling factors , power heuristic is used 
float	MisFactor( int nf, float fPdf, int ng, float gPdf )
{
	float f = nf * fPdf, g = ng * gPdf;
    return (f*f) / (f*f + g*g);
}
