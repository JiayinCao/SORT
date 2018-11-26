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

#include "math/point.h"
#include "geometry/ray.h"
#include "utility/enum.h"
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "utility/strhelper.h"
#include "utility/creator.h"
#include "math/vector2.h"

class PixelSample;
class ImageSensor;
class Visibility;

//! @brief Abstruct camera
/**
 * This class serves as an abstract interface for different camera model.
 * There are several derived classes, such as perspective camera, othogonal 
 * camera and environment camera.
 */
class	Camera : public PropertySet<Camera>
{
public:
	//! @brief Virtual destructor.
    virtual ~Camera() {}

    //! @brief Camera pre-proessing.
    //!
    //! Camera will do some pre-processing after camera intialization once all properties have been set.
    virtual void PreProcess() {}
    
	//! @brief Generating a primary ray.
    //! @param x    Coordinate along horizontal axis on the image sensor, it could be a float value.
    //! @param y    Coordinate along vertical axis on the image sensor, it could be a float value.
    //! @param ps   Pixel sample holding several useful random variables.
    //! @return     The generated ray based on the input.
	virtual Ray	GenerateRay( float x, float y, const PixelSample& ps) const = 0;

	//! @brief Setup image sensor for the camera.
    //! @param is   The pointer to an existed image sensor.
	void SetImageSensor(ImageSensor* is) { m_imagesensor = is; }

    //! @brief Get attached image sensor.
    //! @return The attached image sensor.
    ImageSensor* GetImageSensor() {
        return m_imagesensor;
    }

	//! @brief Get camera viewing point.
    //! @return Viewing point of the camera.
	const Point& GetEye() const { return m_eye; }
    
    //! @brief Setup viewing point for the camera.
    //! @param eye  Viewing point to set up in the camera.
	virtual void SetEye(const Point& eye) { m_eye = eye; }

	//! @brief Get viewing direction.
    //! @return Camera forward direction.
	virtual Vector GetForward() const = 0;

    //! @brief Get camera coordinate according to a view direction in world space. It is used in light tracing or bi-directional path tracing algorithm.
    //! @param p                A point in world space. The calculation will connect it to the viewing point of the cammere seeking the intersected point between the direction and the image sensor.
    //! @param pdfw             PDF w.r.t the solid angle of choosing the direction.
    //! @param pdfa             PDF w.r.t the area of choosing the viewing point.
    //! @param cosAtCamera      The cosine factor of the angle between the viewing direction and forward direction.
    //! @param we               The importance function.
    //! @param eyeP             The selected random viewing point in world space.
    //! @param visibility       The structure holding visibility information.
    //! @return                 The coordinate on the image sensor. Its values range from 0 to width/height - 1.
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility) const = 0;

protected:
	Point           m_eye;                      /**< Viewing point of the camera. */
    ImageSensor*    m_imagesensor = nullptr;    /**< Image sensor. */
	CAMERA_TYPE     m_type = CT_NONE;           /**< Camera type. */
    float           m_sensorW = 0.0f;           /**< Image sensor width. */
    float           m_sensorH = 0.0f;           /**< Image sensor height. */
    float           m_aspectRatioW = 0.0f;      /**< Aspect ratio along x axis. */
    float           m_aspectRatioH = 0.0f;      /**< Aspect ratio along y axis. */
	int             m_aspectFit = 0;            /**< Aspect fit. It equals to 1 if it fits horizontally, otherwise it is 2. */

	// property handler
	class EyeProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(EyeProperty,Camera);
		
		// set value
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
		{
			Camera* camera = CAST_TARGET(Camera);

			std::string _str = str;
			std::string x = NextToken( _str , ' ' );
			std::string y = NextToken( _str , ' ' );

			camera->m_aspectRatioW = (float)atof( x.c_str() );
			camera->m_aspectRatioH = (float)atof( y.c_str() );
		}
	};
};
