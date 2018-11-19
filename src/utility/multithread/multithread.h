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

#include "sort.h"

// get the number of cpu cores in the system
unsigned NumSystemCores();

// get the thread id
int ThreadId();

#include <list>
#include <memory>
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
    unsigned		taskId = 0;
    bool*			taskDone = nullptr;	// used to show the progress
    
    // the pixel sample
    PixelSample*	pixelSamples = nullptr;
    unsigned		samplePerPixel = 0;
    
    // the sampler
    Sampler*		sampler = nullptr;
    // the camera
    Camera*			camera = nullptr;
    // the scene description
    const Scene&	scene;
    
    // constructor
    RenderTask( Scene& sc , Sampler* samp , Camera* cam , bool* td, unsigned spp )
    :taskDone(td),samplePerPixel(spp),sampler(samp),camera(cam),scene(sc)
    {
    }
    
    // execute the task
    void Execute( std::shared_ptr<Integrator> integrator );
    
    static void DestoryRenderTask( RenderTask& rt )
    {
        SAFE_DELETE_ARRAY(rt.pixelSamples);
    }
};

class RenderTaskQueue : public Singleton<RenderTaskQueue>
{
public:
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
    
private:
    std::list<RenderTask> m_taskList;
    
    // private constructor
    RenderTaskQueue(){}
    
    friend class Singleton<RenderTaskQueue>;
};

#include "stdthread.h"
