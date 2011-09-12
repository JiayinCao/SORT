/*
   FileName:      checkboxtexture.h

   Created Time:  2011-08-04 12:44:25

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_CHECKBOXTEXTURE
#define	SORT_CHECKBOXTEXTURE

// include the header
#include "texture.h"
#include "utility/strhelper.h"

//////////////////////////////////////////////////////////////////////////////////////
//	definition of check box texture
class	CheckBoxTexture : public Texture
{
// public funcion
public:
	DEFINE_CREATOR( CheckBoxTexture );

	// default constructor
	CheckBoxTexture();
	// constructor from two colors
	// para 'c0' :	color0
	// para 'c1' :	color1
	CheckBoxTexture( const Spectrum& c0 , const Spectrum& c1 );
	// constructor from six float
	// para 'r0' :	r component of the first color
	// para 'g0' :	g component of the first color
	// para 'b0' :	b component of the first color
	// para 'r1' :	r component of the second color
	// para 'g1' :	g component of the second color
	// para 'b1' :	b component of the second color
	CheckBoxTexture( 	float r0 , float g0 , float b0 ,
					float r1 , float g1 , float b1 );
	// destructor
	~CheckBoxTexture();

	// get the texture value
	// para 'x'	:	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	virtual Spectrum GetColor( int x , int y ) const;

// private field
private:
	// two colors
	Spectrum	m_Color0;
	Spectrum	m_Color1;

// private method
private:
	// init default values
	void	_init();

	// register properties
	void _registerAllProperty();

// property handler
	class Color0Property : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(Color0Property,Texture);

		// set value
		void SetValue( const string& value )
		{
			CheckBoxTexture* ct = CAST_TARGET(CheckBoxTexture);
			ct->m_Color0 = SpectrumFromStr( value );
		}
	};
	class Color1Property : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(Color1Property,Texture);

		// set value
		void SetValue( const string& value )
		{
			CheckBoxTexture* ct = CAST_TARGET(CheckBoxTexture);
			ct->m_Color1 = SpectrumFromStr( value );
		}
	};
};

#endif
