/*
   FileName:      perspective.h

   Created Time:  2011-08-04 12:52:11

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_PERSPECTIVE
#define	SORT_PERSPECTIVE

// include the header file
#include "camera.h"

////////////////////////////////////////////////////////////////////////////////////
//	definition of the perpective camera
class	PerspectiveCamera : public Camera
{
// public method
public:

	DEFINE_CREATOR( PerspectiveCamera , "perspective" );

	// default constructor
	PerspectiveCamera(){_init();}
	// destructor
	~PerspectiveCamera(){}

    // Preprocess
    virtual void PreProcess();
    
	// generate ray
	virtual Ray GenerateRay( float x , float y , const PixelSample& ps ) const;

	// get and set target
	const Point& GetTarget() const { return m_target; }
	void SetTarget( const Point& t ) { m_target = t; }

	// get and set up
	const Vector& GetUp() const { return m_up; }
	void SetUp( const Vector& u ) { m_up = u; }

	// get and set fov
	float GetFov() const { return m_fov; }
	void SetFov( float fov ) { m_fov = fov; }

	// set len
	void SetLen( float len )
	{ m_lensRadius = len; }
	// get len
	float GetLen() const { return m_lensRadius; }
	
	// get camera coordinate according to a view direction in world space
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility ) const;
	
	// get eye direction
	virtual Vector GetForward() const {
        return m_forward;
    }

// protected field
protected:
	// the target of the camera
	Point m_target;
	// the up vector of the camera
	Vector m_up;
    // forward direction
    Vector m_forward;

	// the fov for the camera
	float  m_fov;
	
	// radius for the lens , zero, by default, means no dof
	float m_lensRadius;

	// image distance with each pixel area equals to exactly one
	float m_imagePlaneDist;
    
    // focal distance
    float m_focalDistance;

	// inverse aparture size
	float m_inverseApartureSize;

    // Transformation
    Transform   m_cameraToClip;
    Transform   m_clipToRaster;
    Transform   m_cameraToRaster;
    Transform   m_worldToCamera;
    Transform   m_worldToRaster;
	
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
			PerspectiveCamera* camera = CAST_TARGET(PerspectiveCamera);
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
			PerspectiveCamera* camera = CAST_TARGET(PerspectiveCamera);
			camera->SetTarget( PointFromStr(str) );
		}
	};
	// property handler
	class FovProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(FovProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			PerspectiveCamera* camera = CAST_TARGET(PerspectiveCamera);
			camera->SetFov( (float)atof(str.c_str()) );
		}
	};
	// property handler
	class LenProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(LenProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			PerspectiveCamera* camera = CAST_TARGET(PerspectiveCamera);
			camera->SetLen( (float)atof(str.c_str()) );
		}
	};
};

#endif
