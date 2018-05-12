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

// include header
#include "distant.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

IMPLEMENT_CREATOR( DistantLight );

// sample a ray
Spectrum DistantLight::sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const
{
    const Vector light_dir = Vector3f( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
    
	// distant light direction
	dirToLight = -light_dir;
    
	if( pdfw )
        *pdfw = 1.0f;
    
    if( distance )
        *distance = 1e6f;
    
    if( emissionPdf )
    {
        const BBox& box = scene->GetBBox();
        Vector delta = box.m_Max - box.m_Min;
        *emissionPdf = 4.0f * INV_PI / delta.SquaredLength();
    }
    
    if( cosAtLight )
        *cosAtLight = 1.0f;

    const float delta = 0.01f;
	visibility.ray = Ray( intersect.intersect , dirToLight , 0 , delta );

	return intensity;
}

// sample a ray from light
Spectrum DistantLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const
{
    const Vector light_dir = Vector3f( light2world.matrix.m[1] , light2world.matrix.m[5] , light2world.matrix.m[9] );
    
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
    r.m_Dir = light_dir;
	
	const BBox& box = scene->GetBBox();
	const Point center = ( box.m_Max + box.m_Min ) * 0.5f;
	const Vector delta = box.m_Max - center;
	const float world_radius = delta.Length();

	Vector v0 , v1;
	CoordinateSystem( r.m_Dir , v0 , v1 );
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );
	const Point p = ( u * v0 + v * v1 ) * world_radius + center;
	r.m_Ori = p - r.m_Dir * world_radius * 3.0f;

    const float pdf = 1.0f / ( PI * world_radius * world_radius );
	if( pdfW ) *pdfW = pdf;
	if( pdfA ) *pdfA = 1.0f;
    if( cosAtLight ) *cosAtLight = 1.0f;
    
	return intensity;
}
