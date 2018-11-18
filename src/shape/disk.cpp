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

// include header file
#include "disk.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

// sample a point on shape
Point Disk::Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const
{
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );

	Point lp = m_transform( Point( u * radius , 0.0f , v * radius ) );
    n = m_transform( Vector( 0 , 1.0f , 0 ) );
	Vector delta = lp - p;
	wi = Normalize( delta );

	float dot = Dot( -wi , n );
	if( pdf )
	{
		if( dot <= 0.0f )
			*pdf = 0.0f;
		else
			*pdf = delta.SquaredLength() / ( SurfaceArea() * dot );
	}

	return lp;
}

// sample a ray from light
void Disk::Sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const
{
	float u , v;
	UniformSampleDisk( ls.u , ls.v , u , v );
	r.m_fMin = 0.0f;
	r.m_fMax = FLT_MAX;
	r.m_Ori = m_transform(Point( u * radius , 0.0f , v * radius ));
	Vector wi = UniformSampleHemisphere(sort_canonical() , sort_canonical());
	r.m_Dir = m_transform(wi);
	n = m_transform.invMatrix.Transpose()( DIR_UP );

	if( pdf ) *pdf = 1.0f / ( radius * radius * PI * TWO_PI );
}

// the surface area of the shape
float Disk::SurfaceArea() const
{
	return PI * radius * radius;
}

// get intersected point between the ray and the shape
bool Disk::GetIntersect( const Ray& r , Point& p , Intersection* intersect ) const
{   
	Ray ray = m_transform.invMatrix( r );

	if( ray.m_Dir.y == 0.0f )
		return false;

	const float limit = intersect ? intersect->t : FLT_MAX;
	
	float t = -ray.m_Ori.y / ray.m_Dir.y;
	if( t > limit || t <= ray.m_fMin || t > ray.m_fMax )
		return false;
	p = ray(t);
	float sqLength = p.x * p.x + p.z * p.z;
	if( sqLength > radius * radius )
		return false;

	if( intersect )
	{
		intersect->t = t;
		intersect->intersect = m_transform( p );
		intersect->normal = m_transform.invMatrix.Transpose()(DIR_UP);
		intersect->tangent = m_transform(Vector( 0.0f , 0.0f , 1.0f ));
	}

	return true;
}

// get the bounding box of the primitive
const BBox&	Disk::GetBBox() const
{
	if( !m_bbox )
	{
        m_bbox = std::unique_ptr<BBox>( new BBox() );
		m_bbox->Union( m_transform( Point( radius , 0.0f , radius ) ) );
		m_bbox->Union( m_transform( Point( radius , 0.0f , -radius ) ) );
		m_bbox->Union( m_transform( Point( -radius , 0.0f , radius ) ) );
		m_bbox->Union( m_transform( Point( -radius , 0.0f , -radius ) ) );
	}

	return *m_bbox;
}
