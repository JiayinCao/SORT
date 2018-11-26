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

#include "camera.h"

//! @brief Orthogonal camera.
/**
 * Orthogonal camera does not deliver the effect that faraway objects appear
 * much smaller comparing with nearer objects. It is commonly used in applications
 * like 3ds max, CAD.
 */
class	OrthoCamera : public Camera
{
public:
	DEFINE_CREATOR( OrthoCamera , Camera , "ortho" );

	//! @brief Default constructor
	OrthoCamera();

    //! @brief Generating a primary ray.
    //! @param x    Coordinate along horizontal axis on the image sensor, it could be a float value.
    //! @param y    Coordinate along vertical axis on the image sensor, it could be a float value.
    //! @param ps   Pixel sample holding several useful random variables.
    //! @return     The generated ray based on the input.
	virtual Ray GenerateRay( float x , float y , const PixelSample& ps ) const;

    //! @brief Get camera viewing target.
    //! @return Camera viewing target.
    const Point& GetTarget() const { return m_target; }
    
    //! @brief Set camera viewing target in world space.
    //! @param t Target point to be set.
    void SetTarget( const Point& t ) { m_target = t; updateTransform(); }
    
    //! @brief Get camera up direction in world space.
    //! @return Camera up direction in world space.
    const Vector& GetUp() const { return m_up; }
    
    //! @brief Set camera up direction in world space.
    //! @param u Camera up direction to be set.
    void SetUp( const Vector& u ) { m_up = u; updateTransform(); }
	
    //! @brief Setup viewing point for the camera.
    //! @param eye  Viewing point to set up in the camera.
    void SetEye( const Point& eye ) { m_eye = eye; updateTransform(); }

	//! @brief Get width of camera image plane in world space.
    //! @return The width of camera image plane in world space.
	float GetCameraWidth() const { return m_camWidth; }
    
    //! @brief Get height of camera image plane in world space.
    //! @return The height of camera image plane in world space.
	float GetCameraHeight() const { return m_camHeight; }
    
    //! @brief Set width of camera image plane in world space.
    //! @param w Width to be set.
	void SetCameraWidth( float w );
    //! @brief Set height of camera image plane in world space.
    //! @param h Height to be set.
	void SetCameraHeight( float h );

    //! @brief Get camera coordinate according to a view direction in world space. It is used in light tracing or bi-directional path tracing algorithm.
    //! @param p                A point in world space. The calculation will connect it to the viewing point of the cammere seeking the intersected point between the direction and the image sensor.
    //! @param pdfw             PDF w.r.t the solid angle of choosing the direction.
    //! @param pdfa             PDF w.r.t the area of choosing the viewing point.
    //! @param cosAtCamera      The cosine factor of the angle between the viewing direction and forward direction.
    //! @param we               The importance function.
    //! @param eyeP             The selected random viewing point in world space.
    //! @param visibility       The structure holding visibility information.
    //! @return                 The coordinate on the image sensor. Its values range from 0 to width/height - 1.
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility) const {
		sAssert(false, CAMERA);
		return Vector2i();
	}

    //! @brief Get viewing direction.
    //! @return Camera forward direction.
	virtual Vector GetForward() const{
		sAssert(false, CAMERA);
		return Vector();
	}

protected:
	Point   m_target;            /**< Viewing target of the camera in world space. */
	Vector  m_up;                /**< Up direction of the camera in world space. */
    float   m_camWidth = 1.0f;   /**< Camera image plane width in world space. */
    float   m_camHeight = 1.0f;  /**< Camera image plane height in world space. */
	Matrix  world2camera;        /**< Transformation from world space to view space. */

    //! @brief Udpate transformation
    void updateTransform();
    
	//! @brief Register all properties.
	void registerAllProperty();
	
	// property handler
	class UpProperty : public PropertyHandler<Camera>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(UpProperty,Camera);
		
		// set value
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
		{
			OrthoCamera* camera = CAST_TARGET(OrthoCamera);
			camera->SetCameraHeight( (float)atof(str.c_str()) );
		}
	};
};
