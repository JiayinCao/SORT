/*
   FileName:      ortho.h

   Created Time:  2011-08-23 13:04:22

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_ORTHO
#define	SORT_ORTHO

// include the header file
#include "camera.h"
#include "math/transform.h"

////////////////////////////////////////////////////////////////////////////////////
//	definition of the ortho camera
class	OrthoCamera : public Camera
{
// public method
public:
	DEFINE_CREATOR( OrthoCamera , "ortho" );

	// default constructor
	OrthoCamera();
	// destructor
	~OrthoCamera(){}

	// generate ray
	virtual Ray GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const;

	// get and set target
	const Point& GetTarget() const { return m_target; }
	void SetTarget( const Point& t ) { m_target = t; _updateTransform(); }

	// get and set up
	const Vector& GetUp() const { return m_up; }
	void SetUp( const Vector& u ) { m_up = u; _updateTransform(); }
	
	// set up eye point
	virtual void SetEye( const Point& eye ) { m_eye = eye; _updateTransform(); }

	// set the width and height for the camera
	float GetCameraWidth() const { return m_camWidth; }
	float GetCameraHeight() const { return m_camHeight; }
	void SetCameraWidth( float w );
	void SetCameraHeight( float h );

// protected field
protected:
	// the target of the camera
	Point m_target;
	// the up vector of the camera
	Vector m_up;

	// set the width and height for the camera
	float m_camWidth , m_camHeight;

	// the transformation
	Matrix world2camera;

	// update transform matrix
	void _updateTransform();
	
	// initialize data
	void _init();
	
	// register all properties
	void _registerAllProperty();
	
	// property handler
	class UpProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(UpProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			OrthoCamera* camera = CAST_TARGET(OrthoCamera);
			camera->SetUp( VectorFromStr(str) );
		}
	};
	// property handler
	class TargetProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(TargetProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			OrthoCamera* camera = CAST_TARGET(OrthoCamera);
			camera->SetTarget( PointFromStr(str) );
		}
	};
	// property handler
	class WidthProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(WidthProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			OrthoCamera* camera = CAST_TARGET(OrthoCamera);
			camera->SetCameraWidth( (float)atof(str.c_str()) );
		}
	};
	// property handler
	class HeightProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(HeightProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			OrthoCamera* camera = CAST_TARGET(OrthoCamera);
			camera->SetCameraHeight( (float)atof(str.c_str()) );
		}
	};
};

#endif