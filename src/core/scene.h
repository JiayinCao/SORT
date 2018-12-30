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

#include "core/define.h"
#include <vector>
#include "core/sassert.h"
#include "math/bbox.h"
#include "spectrum/spectrum.h"
#include "math/intersection.h"
#include "math/transform.h"
#include "camera/camera.h"
#include "entity/entity.h"
#include "core/primitive.h"
#include "core/samplemethod.h"

class Light;

////////////////////////////////////////////////////////////////////////////
// definition of scene class
class	Scene{
public:
	//! @brief Serialize scene from stream.
	bool	LoadScene( class IStreamBase& root );

	// get the intersection between a ray and the scene
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	// note     : if there is no acceleration structure , it will iterator all
	//			  of the primitives which will cost much!
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;

	// release the memory of the scene
	void	Release();

	// preprocess
	void	PreProcess();

	// get light
	const Light* GetLight( unsigned i ) const{
		sAssert( i < m_lights.size() , LIGHT );
		return m_lights[i];
	}
	// add light
	void AddLight( Light* light ){
		if( light )
			m_lights.push_back( light );
	}
	// get lights
	const std::vector<Light*>& GetLights() const {return m_lights;}
	// get sky light
	const Light* GetSkyLight() const {return m_skyLight;}
    // set sky light
    void SetSkyLight(Light* light){
		m_skyLight = light;
	}
	// get sampled light
	const Light* SampleLight( float u , float* pdf ) const;
	// get the properbility of the sample
	float LightProperbility( unsigned i ) const;
	// get the number of lights
	unsigned LightNum() const{
		return (unsigned)m_lights.size();
	}
	// get bounding box of the scene
	const BBox& GetBBox() const;
    // add primitives
    void AddPrimitive(Primitive* primitive) {
		m_primitiveBuf.push_back( std::unique_ptr<Primitive>(primitive) );
	}
	// get primtives
	const std::vector<std::unique_ptr<Primitive>>*	 GetPrimitives() const {
		return &m_primitiveBuf; 
	}

	// Evaluate sky
	Spectrum	Le( const Ray& ray ) const;

    // Setup scene camera
    void SetupCamera(Camera* camera) { 
		m_camera = camera; 
	}
    // Get camera from the scene
    Camera* GetCamera() const { 
		return m_camera; 
	}

private:
	std::vector<std::unique_ptr<Entity>>		m_entities;		/**< Entities in the scene. */
	std::vector<std::unique_ptr<Primitive>>		m_primitiveBuf; /**< Primitives in the scene. */
	std::vector<Light*>							m_lights;       /**< Lights in the scene. */

	Light*                  m_skyLight = nullptr;   /**< Sky light if available. */
	Camera*                 m_camera = nullptr;     /**< Camera of the scene. */

	/**< distribution of light power */
	std::unique_ptr<Distribution1D>		    	m_lightsDis = nullptr;
	
	// bounding box for the scene
	mutable BBox	m_BBox;

	// brute force intersection test ( it will only invoked if there is no acceleration structure
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	bool	_bfIntersect( const Ray& r , Intersection* intersect ) const;

	// generate triangle buffer
	void	_generatePriBuf();

	// compute light cdf
	void	_genLightDistribution();

	friend class MeshVisual;
};