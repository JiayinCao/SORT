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

#ifndef	SORT_SYSTEM
#define	SORT_SYSTEM

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
// public method
public:
	// default constructor
	System();

	// pre-process before rendering
	void PreProcess();
	// render the image
	void Render();
	// output the render target
	void OutputRT();

	// setup system from file
	bool Setup( const char* str );
	
	// load the scene
	bool LoadScene( const string& str );
	// get scene
	const Scene& GetScene() const { return m_Scene; }

	// get elapsed time
	unsigned GetRenderingTime() const;

	// output log information
	void OutputLog() const;

	// uninitialize
	void Uninit();

	// get resource path
	const string& GetResourcePath() const { return m_ResourcePath; }
	// set resource path
	void SetResourcePath( const string& str ) { m_ResourcePath = str; }

//private field:
private:
    // image sensor
    ImageSensor*    m_imagesensor;

	// the camera for the system
	Camera*			m_camera;

	unsigned		m_totalTask;
	bool*			m_taskDone;
	char*			m_pProgress;

	// the integrator type
	string			m_integratorType;
	// integrator properties
	struct Property
	{
		string _name;
		string _property;
	};
	vector<Property>	m_integratorProperty;
	// the scene for rendering
	Scene			m_Scene;
	// the sampler
	Sampler*		m_pSampler;
	// sample number per pixel
	unsigned		m_iSamplePerPixel;

	// rendering time
	unsigned		m_uRenderingTime;
	// pre-processing time
	unsigned		m_uPreProcessingTime;

	// path for the resource
	string			m_ResourcePath;
	string			m_OutputFileName;

	// number of thread to allocate
	unsigned		m_thread_num;

	// pre-Initialize
	void	_preInit();
	// output progress
	void	_outputProgress();
	// uninitialize 3rd party library
	void	_uninit3rdParty();
	// do ray tracing in a multithread enviroment
	void	_executeRenderingTasks();
	// push rendering task
	void	_pushRenderTask();
	// allocate integrator
	Integrator*	_allocateIntegrator();
};

#endif
