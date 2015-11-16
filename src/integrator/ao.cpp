/*
   FileName:      ao.cpp

   Created Time:  2015-11-16

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "ao.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "light/light.h"
#include "math/vector3.h"
#include "utility/samplemethod.h"

IMPLEMENT_CREATOR( AmbientOcclusion );

// radiance along a specific ray direction
Spectrum AmbientOcclusion::Li( const Ray& r , const PixelSample& ps ) const
{
	if( r.m_Depth > max_recursive_depth )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.Le(r).IsBlack()?0.0f:1.0f;

	Vector nn = ip.normal;
	Vector tn = Normalize(Cross( nn , ip.tangent ));
	Vector sn = Cross( tn , nn );
	Vector _wi = CosSampleHemisphere( sort_canonical() , sort_canonical() );

	Vector wi = Vector( _wi.x * sn.x + _wi.y * nn.x + _wi.z * tn.x ,
						_wi.x * sn.y + _wi.y * nn.y + _wi.z * tn.y ,
						_wi.x * sn.z + _wi.y * nn.z + _wi.z * tn.z );

	// the ray to be tested
	Ray ray( ip.intersect , wi , 0 , 0.001f , maxDistance );

	// if there is no intersection, return 1.0 as full illumination
	Intersection aoip;
	if( !scene.GetIntersect( ray , &aoip ) )
		return 1.0f;

	float distance = Distance( aoip.intersect , ray.m_Ori );
	return pow( distance / maxDistance , 4.0f );
}

// output log information
void AmbientOcclusion::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Ambient Occlusion."<<ENDL;
}

void AmbientOcclusion::_registerAllProperty()
{
	_registerProperty( "max_distance" , new MaxDistanceProperty(this) );
}