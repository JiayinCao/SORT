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

#ifndef	SORT_CAMERA
#define	SORT_CAMERA

#include "math/point.h"
#include "geometry/ray.h"
#include "utility/enum.h"
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "utility/strhelper.h"
#include "utility/creator.h"
#include "math/vector2.h"

// pre-decleration of render target
class PixelSample;
class ImageSensor;
class Visibility;

////////////////////////////////////////////////////////////////////
//	definition of camera
class	Camera : public PropertySet<Camera>
{
	// public method
public:
	// destructor
	virtual ~Camera() {}

    // Preprocess
    virtual void PreProcess(){}
    
	// generate a ray given a pixel
	virtual Ray	GenerateRay( float x, float y, const PixelSample& ps) const = 0;

	// set a render target
	void SetImageSensor(ImageSensor* is) { m_imagesensor = is; }

	// get and set eye point
	const Point& GetEye() const { return m_eye; }
	virtual void SetEye(const Point& eye) { m_eye = eye; }

	// get eye direction
	virtual Vector GetForward() const = 0;

	// get image sensor
	ImageSensor* GetImageSensor() {
		return m_imagesensor;
	}

	// get camera coordinate according to a view direction in world space
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility) const = 0;

// protected field
protected:
	// the eye point
	Point	m_eye;
    // the image sensor
    ImageSensor*    m_imagesensor = nullptr;
	// the type for the camera
	CAMERA_TYPE m_type = CT_NONE;
	// the size of the sensor
	float	m_sensorW = 0.0f, m_sensorH = 0.0f;
	// aspect ratio
	float	m_aspectRatioW = 0.0f, m_aspectRatioH = 0.0f;
	// aspect fit
	int		m_aspectFit = 0.0f;
	// camera shift
	float	m_shiftX = 0.0f, m_shiftY = 0.0f;

// private method
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

			Point p = PointFromStr(str);

			camera->m_sensorW = p.x;
			camera->m_sensorH = p.y;
			camera->m_aspectFit = (int)p.z;
		}
	};

	// property handler
	class AspectProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(AspectProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			Camera* camera = CAST_TARGET(Camera);

			string _str = str;
			string x = NextToken( _str , ' ' );
			string y = NextToken( _str , ' ' );

			camera->m_aspectRatioW = (float)atof( x.c_str() );
			camera->m_aspectRatioH = (float)atof( y.c_str() );
		}
	};

	// property handler
	class ShiftProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(ShiftProperty,Camera);
		
		// set value
		void SetValue( const string& str )
		{
			Camera* camera = CAST_TARGET(Camera);

			string _str = str;
			string x = NextToken( _str , ' ' );
			string y = NextToken( _str , ' ' );

			camera->m_shiftX = (float)atof( x.c_str() );
			camera->m_shiftY = (float)atof( y.c_str() );
		}
	};
};

#endif
