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

//! @brief Perspective camera.
/**
 * This is the most commonly used type of camera. It simulates the way
 * human eye and cameras see things.
 */
class	PerspectiveCamera : public Camera
{
public:
	DEFINE_CREATOR( PerspectiveCamera , Camera , "perspective" );

	//! @brief Default constructor.
    PerspectiveCamera(){
        m_type = CT_PERSPECTIVE;
        registerAllProperty();
    }
    
    //! @brief Pre-process after initialization.
    void PreProcess() override;
    
    //! @brief Generating a primary ray.
    //! @param x    Coordinate along horizontal axis on the image sensor, it could be a float value.
    //! @param y    Coordinate along vertical axis on the image sensor, it could be a float value.
    //! @param ps   Pixel sample holding several useful random variables.
    //! @return     The generated ray based on the input.
    Ray GenerateRay( float x , float y , const PixelSample& ps ) const override;

	//! @brief Get camera viewing target.
    //! @return Camera viewing target.
	const Point& GetTarget() const { return m_target; }
    
    //! @brief Set camera viewing target in world space.
    //! @param t Target point to be set.
	void SetTarget( const Point& t ) { m_target = t; }

	//! @brief Get camera up direction in world space.
    //! @return Camera up direction in world space.
	const Vector& GetUp() const { return m_up; }
    
    //! @brief Set camera up direction in world space.
    //! @param u Camera up direction to be set.
	void SetUp( const Vector& u ) { m_up = u; }

	//! @brief Get the field of view for the camera.
    //! @return The field of view for the camera.
	float GetFov() const { return m_fov; }
    
    //! @brief Set the field of view for the camera.
    //! @return The FOV to be set.
	void SetFov( float fov ) { m_fov = fov; }

	//! @brief Set lens radius. A non-zero lens radius means that there is DOF.
    //! @param len The radius of the lens to be set.
	void SetLen( float len )
	{ m_lensRadius = len; }
	
    //! @brief Get lens radius.
    //! return The radius of the lens for the camera.
	float GetLen() const { return m_lensRadius; }
	
    //! @brief Get camera coordinate according to a view direction in world space. It is used in light tracing or bi-directional path tracing algorithm.
    //! @param p                A point in world space. The calculation will connect it to the viewing point of the cammere seeking the intersected point between the direction and the image sensor.
    //! @param pdfw             PDF w.r.t the solid angle of choosing the direction.
    //! @param pdfa             PDF w.r.t the area of choosing the viewing point.
    //! @param cosAtCamera      The cosine factor of the angle between the viewing direction and forward direction.
    //! @param we               The importance function.
    //! @param eyeP             The selected random viewing point in world space.
    //! @param visibility       The structure holding visibility information.
    //! @return                 The coordinate on the image sensor. Its values range from 0 to width/height - 1.
    Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility ) const override;
	
    //! @brief Get viewing direction.
    //! @return Camera forward direction.
	Vector GetForward() const override {
        return m_forward;
    }

protected:
	Point   m_target;                       /**< Viewing target of the camera. */
	Vector  m_up;                           /**< Up direction of the camera. */
    Vector  m_forward;                      /**< Forward direction of the camera. */
    
	float   m_fov = 0.25f;                  /**< Field of view for the camera. */
	float   m_lensRadius = 0.0f;            /**< Radius of the camera lens. */
	float   m_imagePlaneDist = 0.0f;        /**< Distance to the image plane with each pixel equals to exactly one. */
    float   m_focalDistance = 0.0f;         /**< The focal distance for DOF effect. */
	float   m_inverseApartureSize = 0.0f;   /**< Recipocal of the aparture size. */

    Transform   m_cameraToClip;         /**< Transformation from view space to clip space. */
    Transform   m_clipToRaster;         /**< Transformation from clip space to screen space. */
    Transform   m_cameraToRaster;       /**< Transformation from view space to screen space. */
    Transform   m_worldToCamera;        /**< Transformation from world space to camera space. */
    Transform   m_worldToRaster;        /**< Transformation from world space to screen space. */
	
	//! @brief Register all properties for camera.
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
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
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
		void SetValue( const std::string& str )
		{
			PerspectiveCamera* camera = CAST_TARGET(PerspectiveCamera);
			camera->SetLen( (float)atof(str.c_str()) );
		}
	};
};
