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

// include the header file
#include "geometry/scene.h"
#include "integrator/integrator.h"
#include "imagesensor/blenderimage.h"
#include "imagesensor/rendertargetimage.h"

// declare classes
class Camera;
class Sampler;
class PixelSample;
class SORTOutput;

/////////////////////////////////////////////////////////////////////
//	definition of the system
class	System
{
public:
	// pre-process before rendering
	void PreProcess();
	// render the image
	void Render();
	// output the render target
	void OutputRT();

	// setup system from file
	bool Setup( const char* str );
	
	// get scene
	const Scene& GetScene() const { return m_Scene; }

	// uninitialize
	void Uninit();

	// get resource path
	const std::string& GetResourcePath() const { return m_ResourcePath; }
	// set resource path
	void SetResourcePath( const std::string& str ) { m_ResourcePath = str; }

private:
    // image sensor
    ImageSensor*    m_imagesensor = nullptr;

	// the camera for the system
    Camera*			m_camera = nullptr;

    unsigned		m_totalTask = 0;
    bool*			m_taskDone = nullptr;
    char*			m_pProgress = nullptr;

	// the integrator type
	std::string			m_integratorType;
	// integrator properties
	struct Property
	{
		std::string _name;
		std::string _property;
	};
	std::vector<Property>	m_integratorProperty;
	// the scene for rendering
	Scene			m_Scene;
	// the sampler
    Sampler*		m_pSampler = nullptr;
	// sample number per pixel
    unsigned		m_iSamplePerPixel = 0;

	// path for the resource
	std::string			m_ResourcePath;
	std::string			m_OutputFileName;

	// number of thread to allocate
    unsigned		m_thread_num = 1;

	// output progress
	void	_outputProgress();
	// do ray tracing in a multithread enviroment
	void	_executeRenderingTasks();
	// push rendering task
	void	_pushRenderTask();
	// allocate integrator
	Integrator*	_allocateIntegrator();
};
