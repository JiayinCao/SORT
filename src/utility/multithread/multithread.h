/*
	FileName:      multithread.h

	Created Time:  2015-9-17

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_MULTI_THREAD
#define SORT_MULTI_THREAD

#include "sort.h"

// get the number of cpu cores in the system
unsigned NumSystemCores();

// get the thread id
int ThreadId();

#include <list>
#include "utility/singleton.h"
#include "sampler/sample.h"
#include "math/vector2.h"

class Integrator;
class Scene;
class Sampler;
class Camera;
class RenderTarget;
class ImageSensor;

class RenderTask
{
public:
    // the following parameters define where to calculate the image
    Vector2i ori;
	Vector2i size;
    
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
    
    // constructor
    RenderTask( Scene& sc , Sampler* samp , Camera* cam , bool* td, unsigned spp )
    :taskDone(td),samplePerPixel(spp),sampler(samp),camera(cam),scene(sc)
    {
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

#include "stdthread.h"

#endif // SORT_SHAREDMEMORY
