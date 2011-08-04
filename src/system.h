/*
   FileName:      system.h

   Created Time:  2011-08-04 12:42:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SYSTEM
#define	SORT_SYSTEM

// include the header file
#include "geometry/scene.h"
#include "integrator/integrator.h"

// declare classes
class Camera;
class RenderTarget;

/////////////////////////////////////////////////////////////////////
//	definition of the system
class	System
{
// public method
public:
	// default constructor
	System();
	// destructor
	~System();

	// pre-process before rendering
	void PreProcess();
	// render the image
	void Render();
	// output the render target
	void OutputRT( const char* str );

	// load the scene
	bool LoadScene( const string& str );

	// get elapsed time
	unsigned GetRenderingTime() const;

	// output log information
	void OutputLog() const;

//private field:
private:
	// the render target for the system
	RenderTarget*	m_rt;
	// the camera for the system
	Camera*			m_camera;

	// the scene for rendering
	Scene			m_Scene;

	// the integrator for the renderer
	Integrator*		m_pIntegrator;

	// rendering time
	unsigned		m_uRenderingTime;
	// pre-processing time
	unsigned		m_uPreProcessingTime;
	// current rendering pixel id
	unsigned		m_uCurrentPixelId;
	// progress count
	unsigned		m_uProgressCount;
	// previous progress
	unsigned		m_uPreProgress;
	// total pixel number
	unsigned		m_uTotalPixelCount;

	// pre-Initialize
	void	_preInit();
	// post-Uninitialize
	void	_postUninit();
	// output progress
	void	_outputProgress();
};

#endif
