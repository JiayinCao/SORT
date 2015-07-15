/*
   FileName:      taskqueue.h

   Created Time:  2015-07-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_TASKQUEUE
#define SORT_TASKQUEUE

#include <list>
#include "utility\singleton.h"
#include "utility\define.h"
#include "sampler\sample.h"

class Integrator;
class Scene;
class Sampler;
class Camera;
class RenderTarget;

class RenderTask
{
public:
	// the following parameters define where to calculate the image
	unsigned ori_x;
	unsigned ori_y;
	unsigned width;
	unsigned height;

	// the task id
	unsigned		taskId;
	bool*			taskDone;	// used to show the progress

	// the pixel sample
	PixelSample*	pixelSamples;
	unsigned		samplePerPixel;

	// the sampler
	Sampler*		sampler;
	// the camera
	Camera*			camera;
	// the scene description
	const Scene&	scene;
	// the render target
	RenderTarget*	rendertarget;

	// constructor
	RenderTask( Scene& sc , Sampler* samp , Camera* cam , RenderTarget* rt , bool* td, unsigned spp )
		:scene(sc),sampler(samp),camera(cam),samplePerPixel(spp),rendertarget(rt),taskDone(td)
	{
		ori_x = 0;
		ori_y = 0;
		width = 0;
		height = 0;
		pixelSamples = 0;
		taskId = 0;
	}

	// execute the task
	void Execute( Integrator* integrator );
	
	static void DestoryRenderTask( RenderTask& rt )
	{
		SAFE_DELETE_ARRAY(rt.pixelSamples);
	}
};

class RenderTaskQueue : public Singleton<RenderTaskQueue>
{
// public method
public:
	~RenderTaskQueue(){}

	// Add Task
	void PushTask( RenderTask task ){
		m_taskList.push_back(task);
	}

	// Pop task
	RenderTask PopTask(){

		RenderTask t = m_taskList.front();
		m_taskList.pop_front();

		return t;
	}

	// Is the queue empty
	bool IsEmpty() const{
		return m_taskList.empty();
	}

// private field
private:
	std::list<RenderTask> m_taskList;

	// private constructor
	RenderTaskQueue(){}

	friend class Singleton<RenderTaskQueue>;
};

#endif