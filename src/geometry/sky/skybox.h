/*
   FileName:      skybox.h

   Created Time:  2011-08-04 12:51:48

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#pragma once

// include the headers
#include "sky.h"
#include "texture/imagetexture.h"

/////////////////////////////////////////////////////////////////////////////
//	definition of skybox
//	note :	the sampling method in sky box is very ugly, it's better
//			switching to skysphere for better quality and high performance
class	SkyBox : public Sky
{
// public method
public:
	DEFINE_CREATOR( SkyBox , Sky , "sky_box" );

	// default constructor
	SkyBox(){_init();}
	// destructor
	~SkyBox(){_release();}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Vector& r ) const;

	// get the average radiance
	virtual Spectrum GetAverage() const;

	// sample direction
	virtual Vector sample_v( float u , float v , float* pdf , float* area_pdf ) const;

	// get the pdf
	float Pdf( const Vector& wi ) const;

// private field
private:
	// the image textures
	ImageTexture	m_up;
	ImageTexture	m_down;
	ImageTexture	m_front;
	ImageTexture	m_back;
	ImageTexture	m_left;
	ImageTexture	m_right;

	// initialize
	void _init();
	// release
	void _release();
	// register property
	void _registerAllProperty();

// property handler
	class UpProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(UpProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_up.LoadImageFromFile( str );
		}
	};
	class DownProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(DownProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_down.LoadImageFromFile( str );
		}
	};
	class FrontProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(FrontProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_front.LoadImageFromFile( str );
		}
	};
	class BackProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(BackProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_back.LoadImageFromFile( str );
		}
	};
	class LeftProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(LeftProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_left.LoadImageFromFile( str );
		}
	};
	class RightProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(RightProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkyBox* sky = CAST_TARGET(SkyBox);
			sky->m_right.LoadImageFromFile( str );
		}
	};
};
