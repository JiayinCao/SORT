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

// include the header
#include "spot.h"
#include "geometry/intersection.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

IMPLEMENT_CREATOR( SpotLight );

// sample ray from light
Spectrum SpotLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const
{
    const Vector light_dir = Vector3f( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
    const Vector light_pos = Vector3f( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
    
    // direction to light
	const Vector _dirToLight = light_pos - intersect.intersect;
    
    // Normalize vec
    const float sqrLen = _dirToLight.SquaredLength();
    const float len = sqrt(sqrLen);
    dirToLight = _dirToLight / len;
    
    // direction pdf from 'intersect' to light source w.r.t solid angle
	if( pdfw )
        *pdfw = sqrLen;
    
    // emission pdf from light source w.r.t solid angle
    if( emissionPdf )
        *emissionPdf = UniformConePdf( cos_total_range );
    
    if( cosAtLight )
        *cosAtLight = 1.0f;
    
    if( distance )
        *distance = len;
    
    // update visility
    const float delta = 0.01f;
    visibility.ray = Ray( light_pos , -dirToLight , 0 , delta , len - delta );
    
	const float falloff = SatDot( dirToLight , -light_dir );
	if( falloff <= cos_total_range )
		return 0.0f;
	if( falloff >= cos_falloff_start )
		return intensity ;
	const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	if( d == 0.0f )
		return 0.0f;

	return intensity * d * d;
}

// sample a ray from light
Spectrum SpotLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const
{
    const Vector light_dir = Vector3f( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
    const Vector light_pos = Vector3f( light2world.matrix.m[3] , light2world.matrix.m[7] , light2world.matrix.m[11] );
    
    // udpate ray
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = light_pos;
    
    // sample a light direction
	const Vector local_dir = UniformSampleCone( ls.u , ls.v , cos_total_range );
	r.m_Dir = light2world.matrix( local_dir );

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
	if( pdfW )
    {
		*pdfW = UniformConePdf( cos_total_range );
		sAssert( *pdfW != 0.0f , LIGHT );
	}
    // pdf w.r.t surface area
	if( pdfA )
        *pdfA = 1.0f;
    if( cosAtLight )
        *cosAtLight = 1.0f;
	
	const float falloff = SatDot( r.m_Dir , light_dir );
	if( falloff <= cos_total_range )
		return 0.0f;
	if( falloff >= cos_falloff_start )
		return intensity;
	const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
	if( d == 0.0f )
		return 0.0f;

	return intensity * d * d;
}

// register property
void SpotLight::_registerAllProperty()
{
	Light::_registerAllProperty();
	_registerProperty( "falloff_start" , new FalloffStartProperty(this) );
	_registerProperty( "range" , new RangeProperty(this) );
}
