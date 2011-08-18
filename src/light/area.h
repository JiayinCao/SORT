/*
   FileName:      area.h

   Created Time:  2011-08-18 09:51:50

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_AREA
#define	SORT_AREA

#include "light.h"
#include "geometry/trimesh.h"
#include "utility/assert.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	definition of area light
class	AreaLight : public Light
{
// public method
public:
	DEFINE_CREATOR(AreaLight);

	// default constructor
	AreaLight(){_registerAllProperty();}
	// destructor
	~AreaLight(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const;

	// total power of the light
	virtual Spectrum Power() const;

	// it's not a delta light
	bool	IsDelta() const { return false; }

// private field
private:
	// the mesh binded to the area light
	TriMesh*	mesh;

	// register property
	void _registerAllProperty();

	class MeshProperty : public PropertyHandler<Light>
	{
	public:
		MeshProperty(Light* light):PropertyHandler(light){}

		// set value
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);

			Sort_Assert( light->scene != 0 );

			light->mesh = light->scene->GetTriMesh( str );

			if( light->mesh == 0 )
				LOG_WARNING<<"There is no model named \""<<str<<"\" attached to area light."<<ENDL;
			else
				light->mesh->SetEmission( light->intensity );
		}
	};
};

#endif