/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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

	// generate ray
	virtual Ray GenerateRay( float x , float y , const PixelSample& ps ) const;

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

	// get camera coordinate according to a view direction in world space
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility) const {
		Sort_Assert(false);
		return Vector2i();
	}

	// get eye direction
	virtual Vector GetForward() const{
		Sort_Assert(false);
		return Vector();
	}

// protected field
protected:
	// the target of the camera
	Point m_target;
	// the up vector of the camera
	Vector m_up;

	// set the width and height for the camera
	float m_camWidth = 1.0f, m_camHeight = 1.0f;

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
