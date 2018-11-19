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
public:
	DEFINE_CREATOR(AreaLight, Light, "area");

	// default constructor
	AreaLight(){_registerAllProperty();}

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


private:
	// the shape binded to the area light
    Shape*	shape = nullptr;
	// the radius for the shape
    float	sizeX = 1.0f;
    float   sizeY = 1.0f;

	// register property
	void _registerAllProperty();

/*
	Refactoring shape class, disable area light for now.

	class ShapeProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(ShapeProperty,Light);
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			
			SAFE_DELETE( light->shape );
			light->shape = CREATE_TYPE( str , Shape );
            if( light->shape ){
				light->shape->SetSizeX( light->sizeX );
                light->shape->SetSizeY( light->sizeY );
                light->shape->SetTransform(light->light2world);
                light->shape->SetLight(light);
            }
		}
	};
	class SizeXProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(SizeXProperty,Light);
		void SetValue( const string& str )
		{
			AreaLight* light = CAST_TARGET(AreaLight);
			
			light->sizeX = (float)atof( str.c_str() );
            if( light->shape ){
                light->shape->SetSizeX( light->sizeX );
                light->shape->SetSizeY( light->sizeY );
            }
		}
	};
    class SizeYProperty : public PropertyHandler<Light>
    {
    public:
        PH_CONSTRUCTOR(SizeYProperty,Light);
        void SetValue( const string& str )
        {
            AreaLight* light = CAST_TARGET(AreaLight);
            
            light->sizeY = (float)atof( str.c_str() );
            if( light->shape ){
                light->shape->SetSizeX( light->sizeX );
                light->shape->SetSizeY( light->sizeY );
            }
        }
    };
*/
};
