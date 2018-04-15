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

#pragma once

#include "light.h"
#include "utility/sassert.h"
#include "shape/shape.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	definition of area light
class	AreaLight : public Light
{
// public method
public:
	DEFINE_CREATOR(AreaLight, Light, "area");

	// default constructor
	AreaLight(){_init();}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'dirToLight': input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const;

	// sample light density
	virtual Spectrum Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const;

	// get intersection between the light and the ray
	virtual bool Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const;

	// total power of the light
	virtual Spectrum Power() const;

	// it's not a delta light
	bool	IsDelta() const { return false; }

	// the pdf for specific sampled directioin
	// note : none-delta light must overwrite this method
	virtual float Pdf( const Point& p , const Vector& wi ) const;

	// get the shape of light
	virtual Shape* GetShape() const { return shape; }

// private field
private:
	// the shape binded to the area light
	Shape*	shape;
	// the radius for the shape
	float	radius;

	// initialize default value
	void _init();

	// register property
	void _registerAllProperty();

	class PosProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(PosProperty,Light);
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

			if( light->shape )
			{
				light->shape->SetTransform( light->light2world );
				light->shape->BindLight( light );
			}
		}
	};
	class DirProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(DirProperty,Light);
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

			if( light->shape ) 
			{
				light->shape->SetTransform( light->light2world );
				light->shape->BindLight(light);
			}
		}
	};
	class ShapeProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(ShapeProperty,Light);
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
		PH_CONSTRUCTOR(RadiusProperty,Light);
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			
			light->radius = 0.5f * (float)atof( str.c_str() );
			if( light->shape )
				light->shape->SetRadius( light->radius );
		}
	};
};
