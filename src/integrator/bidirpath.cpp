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

#include "sort.h"
#include "bidirpath.h"
#include "geometry/scene.h"
#include "light/light.h"
#include "bsdf/bsdf.h"
#include "integratormethod.h"
#include "camera/camera.h"
#include "imagesensor/imagesensor.h"

SORT_STATS_DEFINE_COUNTER(sTotalLengthPathFromEye)
SORT_STATS_DEFINE_COUNTER(sTotalLengthPathFromLight)
SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Bi-directional Path Tracing", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_AVG_COUNT("Bi-directional Path Tracing", "Average Path Length Starting from Eye", sTotalLengthPathFromEye , sPrimaryRayCount);            // This also counts the case where ray hits sky
SORT_STATS_AVG_COUNT("Bi-directional Path Tracing", "Average Path Length Starting from Lights", sTotalLengthPathFromLight , sPrimaryRayCount);       // This also counts the case where ray hits sky

IMPLEMENT_CREATOR( BidirPathTracing );

// return the radiance of a specific direction
Spectrum BidirPathTracing::Li( const Ray& ray , const PixelSample& ps ) const
{
    SORT_STATS(++sPrimaryRayCount);
    
	// pick a light randomly
	float pdf;
	const Light* light = scene.SampleLight( sort_canonical() , &pdf );
	if( light == 0 || pdf == 0.0f )
		return 0.0f;

	Spectrum li;

	float	light_emission_pdf = 0.0f;
	float	light_pdfa = 0.0f;
	Ray		light_ray;
    float   cosAtLight = 1.0f;
	LightSample light_sample(true);
	Spectrum le = light->sample_l( light_sample , light_ray , &light_emission_pdf , &light_pdfa , &cosAtLight );
	
	//-----------------------------------------------------------------------------------------------------
	// Trace light path from light source
	std::vector<BDPT_Vertex> light_path;
	Ray wi = light_ray;
	double vc = (light->IsDelta())?0.0f: MIS(cosAtLight / light_emission_pdf);
	double vcm = MIS(light_pdfa / light_emission_pdf);
	Spectrum throughput = le * cosAtLight / (light_emission_pdf * pdf);
	float rr = 1.0f;
	while ((int)light_path.size() < max_recursive_depth)
	{
        SORT_STATS(++sTotalLengthPathFromLight);
        
		BDPT_Vertex vert;
		if (false == scene.GetIntersect(wi, &vert.inter))
			break;
        
		const float distSqr = vert.inter.t * vert.inter.t;
		const float cosIn = AbsDot( wi.m_Dir , vert.inter.normal );
		if( light_path.size() > 0 || ( light_path.size() == 0 && !light->IsInfinite() ) )
			vcm *= MIS( distSqr );
		vcm /= MIS( cosIn );
		vc /= MIS( cosIn );

		rr = 1.0f;
		if (throughput.GetIntensity() < 0.01f)
			rr = 0.5f;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.throughput = throughput;
		vert.vcm = vcm;
		vert.vc = vc;
		vert.rr = rr;
		vert.depth = (unsigned)(light_path.size() + 1);

		light_path.push_back(vert);

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: light tracing
		_ConnectCamera( vert , (unsigned)light_path.size() , light );

		// russian roulette
		if (sort_canonical() > rr)
			break;

		float bsdf_pdf;
		Spectrum bsdf_value = vert.bsdf->sample_f(vert.wi, vert.wo, BsdfSample(true), &bsdf_pdf, BXDF_ALL);
		bsdf_pdf *= rr;
		const float cosOut = AbsDot(vert.wo, vert.n);
		throughput *= bsdf_value / bsdf_pdf;

		if (bsdf_pdf == 0 || throughput.IsBlack())
			break;

		const float rev_bsdf_pdfw = vert.bsdf->Pdf( vert.wo , vert.wi ) * rr;
		vc = MIS(cosOut/bsdf_pdf) * ( MIS(rev_bsdf_pdfw) * vc + vcm ) ;
		vcm = MIS(1.0f/bsdf_pdf);

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	//-----------------------------------------------------------------------------------------------------
	// Trace light path from eye point
	const unsigned lps = (const unsigned)light_path.size();
	const unsigned int total_pixel = camera->GetImageSensor()->GetWidth() * camera->GetImageSensor()->GetHeight();
	wi = ray;
	throughput = 1.0f;
	int light_path_len = 0;
	vc = 0.0f;
	vcm = MIS(total_pixel / ray.m_fPdfW);
	rr = 1.0f;
	while (light_path_len <= (int)max_recursive_depth)
	{
        SORT_STATS(++sTotalLengthPathFromEye);
        
		BDPT_Vertex vert;
		vert.depth = light_path_len;
		if (false == scene.GetIntersect(wi, &vert.inter))
		{
			// the following code needs to be modified
			if (scene.GetSkyLight() == light)
			{
				if( vert.depth <= max_recursive_depth && vert.depth > 0 )
				{
					float emissionPdf;
					float directPdfA;
					Spectrum _li = light->Le( vert.inter, -wi.m_Dir , &directPdfA , &emissionPdf ) * throughput / light->PickPDF();
                    float weight = (float)(1.0f / (1.0f + MIS(directPdfA) * vcm + MIS(emissionPdf) * vc));
					li += _li * weight;
				}
			}

			if( vert.depth == 0 )
				li += scene.Le( wi );

			break;
		}
        
		const float distSqr = vert.inter.t * vert.inter.t;
		const float cosIn = AbsDot( wi.m_Dir , vert.inter.normal );
		vcm *= MIS( distSqr );
		vcm /= MIS( cosIn );
		vc /= MIS( cosIn );

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: it hits a light source
		if (vert.inter.primitive->GetLight() == light)
		{
			if( vert.depth > 0 && vert.depth <= max_recursive_depth )
			{
				float emissionPdf;
				float directPdfA;
				Spectrum _li = vert.inter.Le(-wi.m_Dir , &directPdfA , &emissionPdf ) * throughput / pdf;
				li += _li / (float)( 1.0f + MIS( directPdfA ) * vcm + MIS( emissionPdf ) * vc );
			}
			else if( vert.depth == 0 )
				li += vert.inter.Le(-wi.m_Dir) / pdf;
		}
		if( light_tracing_only )
			return li;

		rr = 1.0f;
		if (throughput.GetIntensity() < 0.01f )
			rr = 0.5f;

		vert.p = vert.inter.intersect;
		vert.n = vert.inter.normal;
		vert.wi = -wi.m_Dir;
		vert.bsdf = vert.inter.primitive->GetMaterial()->GetBsdf(&vert.inter);
		vert.throughput = throughput;
		vert.vc = vc;
		vert.vcm = vcm;
		vert.rr = rr;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect light sample first
		li += _ConnectLight(vert, light) / pdf;

		//-----------------------------------------------------------------------------------------------------
		// Path evaluation: connect vertices
		for (unsigned j = 0; j < lps; ++j)
			li += _ConnectVertices( light_path[j] , vert , light );

		++light_path_len;

		// Russian Roulette
		if (sort_canonical() > rr)
			break;

		float bsdf_pdf;
		Spectrum bsdf_value = vert.bsdf->sample_f(vert.wi, vert.wo, BsdfSample(true), &bsdf_pdf, BXDF_ALL);
		bsdf_pdf *= rr;
		const float cosOut = AbsDot(vert.wo, vert.n);
		throughput *= bsdf_value / bsdf_pdf;

		if (bsdf_pdf == 0 || throughput.IsBlack())
			break;

		const float rev_bsdf_pdfw = vert.bsdf->Pdf( vert.wo , vert.wi ) * rr;
		vc = MIS( cosOut / bsdf_pdf ) * ( MIS( rev_bsdf_pdfw ) * vc + vcm );
		vcm = MIS( 1.0f / bsdf_pdf );

		wi = Ray(vert.inter.intersect, vert.wo, 0, 0.001f);
	}

	return li;
}

void BidirPathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
	Integrator::RequestSample( sampler, ps , ps_num );

    sample_per_pixel = ps_num;
}

// connect vertices
Spectrum BidirPathTracing::_ConnectVertices( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 , const Light* light ) const
{
	if( p0.depth + p1.depth >= max_recursive_depth )
		return 0.0f;

	const Vector delta = p0.p - p1.p;
	const float invDistcSqr = 1.0f / delta.SquaredLength();
	const Vector n_delta = delta * sqrt(invDistcSqr);

	const float cosAtP0 = AbsDot( p0.n , n_delta );
	const float cosAtP1 = AbsDot( p1.n , n_delta );
	const Spectrum g = p1.bsdf->f( p1.wi , n_delta ) * p0.bsdf->f( p0.wi , -n_delta ) * invDistcSqr;
	if( g.IsBlack() )
		return 0.0f;

	const float p0_bsdf_pdfw = p0.bsdf->Pdf( p0.wi , -n_delta ) * p0.rr;
	const float p0_bsdf_rev_pdfw = p0.bsdf->Pdf( -n_delta , p0.wi ) * p0.rr;
	const float p1_bsdf_pdfw = p1.bsdf->Pdf( p1.wi , n_delta ) * p1.rr;
	const float p1_bsdf_rev_pdfw = p1.bsdf->Pdf( n_delta , p1.wi ) * p1.rr;

	const float p0_a = p1_bsdf_pdfw * cosAtP0 * invDistcSqr;
	const float p1_a = p0_bsdf_pdfw * cosAtP1 * invDistcSqr;

	const double mis_0 = MIS( p0_a ) * ( p0.vcm + p0.vc * MIS( p0_bsdf_rev_pdfw ) );
	const double mis_1 = MIS( p1_a ) * ( p1.vcm + p1.vc * MIS( p1_bsdf_rev_pdfw ) );

    const float weight = (float)(1.0f / (mis_0 + 1.0f + mis_1));

	const Spectrum li = p0.throughput * p1.throughput * g * weight;
	if( li.IsBlack() )
		return li;

	Visibility visible( scene );
	visible.ray = Ray( p1.p , n_delta  , 0 , 0.001f , delta.Length() - 0.001f );
	if( visible.IsVisible() == false )
		return 0.0f;

	return li;
}

// connect light sample
Spectrum BidirPathTracing::_ConnectLight(const BDPT_Vertex& eye_vertex , const Light* light ) const
{
	if( eye_vertex.depth >= max_recursive_depth )
		return 0.0f;

	// drop the light vertex, take a new sample here
	const LightSample sample(true);
	Vector wi;
	Visibility visibility(scene);
	float directPdfW;
	float emissionPdfW;
	float cosAtLight;
	Spectrum li = light->sample_l(eye_vertex.inter, &sample, wi, 0 , &directPdfW, &emissionPdfW , &cosAtLight , visibility);
	const float cosAtEyeVertex = AbsDot(eye_vertex.n, wi);
	li *= eye_vertex.throughput * eye_vertex.bsdf->f(eye_vertex.wi, wi) / directPdfW;

	if (li.IsBlack())
		return 0.0f;

	if (visibility.IsVisible() == false)
		return 0.0f;

	const float eye_bsdf_pdfw = eye_vertex.bsdf->Pdf( eye_vertex.wi , wi ) * eye_vertex.rr;
	const float eye_bsdf_rev_pdfw = eye_vertex.bsdf->Pdf( wi , eye_vertex.wi ) * eye_vertex.rr;

	const double mis0 = light->IsDelta()?0.0f:MIS(eye_bsdf_pdfw / directPdfW);
	const double mis1 = MIS( cosAtEyeVertex * emissionPdfW / ( cosAtLight * directPdfW ) ) * ( eye_vertex.vcm + eye_vertex.vc * MIS( eye_bsdf_rev_pdfw ) );

    const float weight = (float)(1.0f / (mis0 + mis1 + 1.0f));

	return li * weight;
}

// connect camera point
void BidirPathTracing::_ConnectCamera(const BDPT_Vertex& light_vertex, int len , const Light* light ) const
{
	if( light_vertex.depth > max_recursive_depth )
		return;

	Visibility visible( scene );
	float camera_pdfA;
	float camera_pdfW;
	float cosAtCamera;
	Spectrum we;
	Point eye_point;
	const Vector2i coord = camera->GetScreenCoord(light_vertex.inter.intersect, &camera_pdfW, &camera_pdfA , &cosAtCamera , &we , &eye_point , &visible );

	const Vector delta = light_vertex.inter.intersect - eye_point;
	const float invSqrLen = 1.0f / delta.SquaredLength();
	const Vector n_delta = delta * sqrt(invSqrLen);
	
	if( Dot( delta , camera->GetForward() ) <= 0.0f )
		return;

	if (coord.x < 0.0f || coord.y < 0.0f ||
		coord.x >= (int)camera->GetImageSensor()->GetWidth() ||
		coord.y >= (int)camera->GetImageSensor()->GetHeight() ||
		camera_pdfW == 0.0f )
		return;

	const Spectrum bsdf_value = light_vertex.bsdf->f( light_vertex.wi , -n_delta );
	if( bsdf_value.IsBlack() )
		return;

	if( visible.IsVisible() == false )
		return;

	const float total_pixel = (float)(camera->GetImageSensor()->GetWidth() * camera->GetImageSensor()->GetHeight());
	const float gterm = cosAtCamera * invSqrLen;
	Spectrum radiance = light_vertex.throughput * bsdf_value * we * gterm / (float)( sample_per_pixel * total_pixel * camera_pdfA );

	if( !light_tracing_only )
	{
		const float lightvert_pdfA = camera_pdfW * AbsDot( light_vertex.n, n_delta ) * invSqrLen ;
		const float bsdf_rev_pdfw = light_vertex.bsdf->Pdf( -n_delta , light_vertex.wi ) * light_vertex.rr;
		const double mis0 = ( light_vertex.vcm + light_vertex.vc * MIS( bsdf_rev_pdfw ) ) * MIS( lightvert_pdfA / total_pixel );
        const float weight = (float)(1.0f / (1.0f + mis0));

		radiance *= weight;
	}

	// update image sensor
	ImageSensor* is = camera->GetImageSensor();
	if (!is)
		return;
	is->UpdatePixel(coord.x , coord.y , radiance);
}
