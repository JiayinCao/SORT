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
#include "utility/assert.h"
#include "shape/shape.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	definition of area light
class	AreaLight : public Light
{
// public method
public:
	DEFINE_CREATOR(AreaLight);

	// default constructor
	AreaLight(){_init();}
	// destructor
	~AreaLight(){_release();}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const;

	// total power of the light
	virtual Spectrum Power() const;

	// it's not a delta light
	bool	IsDelta() const { return false; }

	// sample light density
	virtual Spectrum sample_l( const Intersection& intersect , const Vector& wo ) const;

	// the pdf for specific sampled directioin
	// note : none-delta light must overwrite this method
	virtual float Pdf( const Point& p , const Vector& wi ) const;

	// get intersection between the light and the ray
	virtual bool Evaluate( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const;

// private field
private:
	// the shape binded to the area light
	Shape*	shape;
	// the radius for the shape
	float	radius;

	// initialize default value
	void _init();
	// release
	void _release();

	// register property
	void _registerAllProperty();

	class PosProperty : public PropertyHandler<Light>
	{
	public:
		PosProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			Point p = PointFromStr( str );
			light->light2world.matrix.m[3] = p.x;
			light->light2world.matrix.m[7] = p.y;
			light->light2world.matrix.m[11] = p.z;
			light->light2world.invMatrix.m[3] = -( light->light2world.invMatrix.m[0] * p.x + 
				light->light2world.invMatrix.m[1] * p.y + light->light2world.invMatrix.m[2] * p.z );
			light->light2world.invMatrix.m[7] = -( light->light2world.invMatrix.m[4] * p.x + 
				light->light2world.invMatrix.m[5] * p.y + light->light2world.invMatrix.m[6] * p.z );
			light->light2world.invMatrix.m[11] = -( light->light2world.invMatrix.m[8] * p.x + 
				light->light2world.invMatrix.m[9] * p.y + light->light2world.invMatrix.m[10] * p.z );

			if( light->shape ) light->shape->SetTransform( light->light2world );
		}
	};
	class DirProperty : public PropertyHandler<Light>
	{
	public:
		DirProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
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
			inv.m[3] = -( light->light2world.invMatrix.m[0] * m.m[3] + 
				light->light2world.invMatrix.m[1] * m.m[7] + light->light2world.invMatrix.m[2] * m.m[11] );
			inv.m[7] = -( light->light2world.invMatrix.m[4] * m.m[3] + 
				light->light2world.invMatrix.m[5] * m.m[7] + light->light2world.invMatrix.m[6] * m.m[11] );
			inv.m[11] = -( light->light2world.invMatrix.m[8] * m.m[3] + 
				light->light2world.invMatrix.m[9] * m.m[7] + light->light2world.invMatrix.m[10] * m.m[11] );

			if( light->shape ) light->shape->SetTransform( light->light2world );
		}
	};
	class ShapeProperty : public PropertyHandler<Light>
	{
	public:
		ShapeProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			
			SAFE_DELETE( light->shape );
			light->shape = CREATE_TYPE( str , Shape );
			if( light->shape )
				light->shape->SetRadius( light->radius );
		}
	};
	class RadiusProperty : public PropertyHandler<Light>
	{
	public:
		RadiusProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			
			light->radius = (float)atof( str.c_str() );
			if( light->shape )
				light->shape->SetRadius( light->radius );
		}
	};
};

#endif
