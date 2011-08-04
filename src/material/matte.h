/*
   FileName:      matte.h

   Created Time:  2011-08-04 12:46:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MATTE
#define	SORT_MATTE

// include header file
#include "material.h"
#include "texture/texture.h"
#include "utility/strhelper.h"

//////////////////////////////////////////////////////////////////////
// definition of matte material
class Matte : public Material
{
// public method
public:
	DEFINE_CREATOR( Matte );

	// default constructor
	Matte();
	// destructor
	~Matte();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

// private field
private:
	// the diffuse color for the material
	Texture* m_d;

	// a scaled color , default is ( 1 , 1 , 1 )
	Spectrum m_scale;

	// register property
	void _registerAllProperty();

	// initialize default value and register property
	void _init();

// property handler
	class ColorProperty : public PropertyHandler<Material>
	{
	public:
		// constructor
		ColorProperty(Material* matte):PropertyHandler(matte){}

		// set value
		void SetValue( Texture* tex )
		{
			Matte* matte = CAST_TARGET(Matte);
			SAFE_DELETE( matte->m_d );
			matte->m_d = tex;
		}
	};
	
	class ScaleProperty : public PropertyHandler<Material>
	{
	public:
		// constructor
		ScaleProperty( Material* matte ) : PropertyHandler(matte){}

		// set value
		void SetValue( const string& str )
		{
			Matte* matte = CAST_TARGET(Matte);
			matte->m_scale = SpectrumFromStr( str );
		}
	};
};

#endif
