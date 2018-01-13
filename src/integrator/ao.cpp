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
#include "ao.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "light/light.h"
#include "math/vector3.h"
#include "utility/samplemethod.h"
#include "log/log.h"

IMPLEMENT_CREATOR( AmbientOcclusion );

// radiance along a specific ray direction
Spectrum AmbientOcclusion::Li( const Ray& r , const PixelSample& ps ) const
{
	if( r.m_Depth > max_recursive_depth )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return 0.0f;

	//return Spectrum((ip.normal.x + 1.0f)*0.5f, (ip.normal.y+1.0f)*0.5f, (ip.normal.z + 1.0f) * 0.5f);
	Vector nn = ip.normal;
	Vector tn = Normalize(Cross( nn , ip.tangent ));
	Vector sn = Cross( tn , nn );
	Vector _wi = CosSampleHemisphere( sort_canonical() , sort_canonical() );

	Vector wi = Vector( _wi.x * sn.x + _wi.y * nn.x + _wi.z * tn.x ,
						_wi.x * sn.y + _wi.y * nn.y + _wi.z * tn.y ,
						_wi.x * sn.z + _wi.y * nn.z + _wi.z * tn.z );

	// flip the incident direction if necessary
	if (Dot(r.m_Dir, nn)>0.0f)
		wi *= -1.0f;

	// the ray to be tested
	Ray ray( ip.intersect , wi , 0 , 0.001f , maxDistance );

	// if there is no intersection, return 1.0 as full illumination
	Intersection aoip;
	if( !scene.GetIntersect( ray , &aoip ) )
		return 1.0f;

	float distance = Distance( aoip.intersect , ray.m_Ori );
	float ao = distance / maxDistance;
	return ao * ao;
}

// output log information
void AmbientOcclusion::OutputLog() const{
    slog( LOG_LEVEL::INFO , LOG_TYPE::INTEGRATOR , "Integrator algorithm : Ambient Occlusion." );
}

void AmbientOcclusion::_registerAllProperty()
{
	_registerProperty( "max_distance" , new MaxDistanceProperty(this) );
}
