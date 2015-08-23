/*
   FileName:      camera.h

   Created Time:  2011-08-04 12:51:59

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_CAMERA
#define	SORT_CAMERA

#include "geometry/point.h"
#include "geometry/ray.h"
#include "utility/enum.h"
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "utility/strhelper.h"
#include "utility/creator.h"

// pre-decleration of render target
class RenderTarget;
class PixelSample;

////////////////////////////////////////////////////////////////////
//	definition of camera
class	Camera : public PropertySet<Camera>
{
// public method
public:
	// default constructor
	Camera(){ _init(); }
	// destructor
	virtual ~Camera(){}

	// generate a ray given a pixel
	virtual Ray	GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const = 0;

	// set a render target
	void SetRenderTarget( RenderTarget* rt ) { m_rt = rt; }

	// get and set eye point
	const Point& GetEye() const { return m_eye; }
	virtual void SetEye( const Point& eye ) { m_eye = eye; }
	
	// get pass number. By default , all of the cameras have only one pass except stereo vision is enabled in perspective camera.
	virtual unsigned GetPassCount() const { return 1; }
	// get pass filter
	virtual Spectrum GetPassFilter( unsigned id ) const { return 1.0f; }

// protected field
protected:
	// the eye point
	Point	m_eye;
	// the render target
	RenderTarget* m_rt;
	// the type for the camera
	CAMERA_TYPE m_type;
	// the size of the sensor
	float	m_SensorW, m_SensorH;

// private method
	// initialize default data
	void _init() { m_rt = 0; m_type = CT_NONE; m_SensorW = 0; m_SensorH = 0; }
	
	// property handler
	class EyeProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(EyeProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			Camera* camera = CAST_TARGET(Camera);
			camera->SetEye( PointFromStr(str) );
		}
	};

	// property handler
	class SensorSizeProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(SensorSizeProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			Camera* camera = CAST_TARGET(Camera);

			string _str = str;
			string x = NextToken( _str , ' ' );
			string y = NextToken( _str , ' ' );

			camera->m_SensorW = (float)atof( x.c_str() );
			camera->m_SensorH = (float)atof( y.c_str() );
		}
	};
};

#endif
