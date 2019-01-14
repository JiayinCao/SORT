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

#include "area.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

Spectrum AreaLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfW , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const{
	sAssert( ls != nullptr , LIGHT );
	sAssert( m_shape != nullptr , LIGHT );

    // sample a point from light
    Vector normal;
	const Point ps = m_shape->Sample_l( *ls , intersect.intersect , dirToLight , normal , pdfW );
    
    // get the delta
    const Vector dlt = ps - intersect.intersect;
    const float len = dlt.Length();
    
    // return if pdf is zero
	if( pdfW && *pdfW == 0.0f )
		return 0.0f;
    
    if(cosAtLight)
        *cosAtLight = Dot( -dirToLight , normal );
    
    if( distance )
        *distance = len;
    
    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
    if( emissionPdf )
        *emissionPdf = UniformHemispherePdf() / m_shape->SurfaceArea();

	// setup visibility tester
    const float delta = 0.01f;
	visibility.ray = Ray( intersect.intersect , dirToLight , 0 , delta , len - delta );

	return intensity;
}

Spectrum AreaLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const{
    sAssert( m_shape != nullptr , LIGHT );
    Vector n;
    m_shape->Sample_l( ls , r , n , pdfW );
    
    if( pdfA )
        *pdfA = 1.0f / m_shape->SurfaceArea();
    
    if( cosAtLight )
        *cosAtLight = SatDot( r.m_Dir , n );
    
    // to avoid self intersection
    r.m_fMin = 0.01f;
    
    return intensity;
}

float AreaLight::Pdf( const Point& p , const Vector& wi ) const{
	sAssert(m_shape != nullptr, LIGHT);
	return m_shape->Pdf( p , wi );
}

Spectrum AreaLight::Power() const{
	sAssert( m_shape != nullptr, LIGHT );
	return m_shape->SurfaceArea() * intensity.GetIntensity() * TWO_PI;
}

Spectrum AreaLight::Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const{
	const float cos = SatDot( wo , intersect.normal );
	if( cos == 0.0f )
		return 0.0f;

	if( directPdfA )
		*directPdfA = 1.0f / m_shape->SurfaceArea();

	if( emissionPdf )
        *emissionPdf = UniformHemispherePdf() / m_shape->SurfaceArea();

	return intensity;
}

bool AreaLight::Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const{
	sAssert( m_shape != nullptr , LIGHT );

	// get intersect
	const auto result = m_shape->GetIntersect( ray , intersect );

	// transform the intersection result back to world coordinate
	if( result && intersect != nullptr )
		radiance = Le( *intersect , -ray.m_Dir , 0 , 0 );

	return result;
}
