/*
   FileName:      distant.h

   Created Time:  2011-08-04 12:48:26

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_DISTANT
#define	SORT_DISTANT

#include "light.h"
#include "geometry/scene.h"

//////////////////////////////////////////////////////////////////////
// definition of distant
class DistantLight : public Light
{
// public method
public:
	DEFINE_CREATOR(DistantLight);
	
	// default constructor
	DistantLight(){_registerAllProperty();}
	// destructor
	~DistantLight(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const;

	// total power of the light
	virtual Spectrum Power() const
	{
		Sort_Assert( scene != 0 );
		const BBox& box = scene->GetBBox();
		float delta = (box.m_Max - box.m_Min).SquaredLength();
		return delta * PI * intensity;
	}

// private field
private:

	// register all properties
	void _registerAllProperty();
	class DirProperty : public PropertyHandler<Light>
	{
	public:
		DirProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			DistantLight* light = CAST_TARGET(DistantLight);
			Vector dir = Normalize(VectorFromStr( str ));
			Vector t0 , t1;
			CoordinateSystem( dir, t0 , t1 );
			Matrix& m = light->light2world.matrix;
			Matrix& inv = light->light2world.invMatrix;
			m.m[0] = t0.x; m.m[1] = dir.x; m.m[2] = t1.x;
			m.m[4] = t0.y; m.m[5] = dir.y; m.m[6] = t1.y;
			m.m[8] = t0.z; m.m[9] = dir.z; m.m[10] = t1.z;
			inv.m[0] = t0.x; inv.m[1] = t0.y; inv.m[2] = t0.z;
			inv.m[4] = dir.x; inv.m[5] = dir.y; inv.m[6] = dir.z;
			inv.m[8] = t1.x; inv.m[9] = t1.y; inv.m[10] = t1.z;
		}
	};
};

#endif
