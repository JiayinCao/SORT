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

// include the header file
#include <time.h>
#include "system.h"
#include "core/log.h"
#include "managers/texmanager.h"
#include "managers/meshmanager.h"
#include "managers/matmanager.h"
#include "managers/memmanager.h"
#include "core/timer.h"
#include "math/intersection.h"
#include "core/path.h"
#include "core/creator.h"
#include "camera/camera.h"
#include "integrator/integrator.h"
#include "sampler/random.h"
#include "managers/smmanager.h"
#include "math/vector2.h"
#include "shape/shape.h"
#include "core/stats.h"
#include "core/profile.h"
#include "task/render_task.h"
#include "core/globalconfig.h"
#include "stream/fstream.h"
#include "core/globalconfig.h"

SORT_STATS_DEFINE_COUNTER(sPreprocessTimeMS)
SORT_STATS_DEFINE_COUNTER(sRenderingTimeMS)
SORT_STATS_DEFINE_COUNTER(sSamplePerPixel)
SORT_STATS_DEFINE_COUNTER(sThreadCnt)

SORT_STATS_TIME("Performance", "Pre-processing Time", sPreprocessTimeMS);
SORT_STATS_TIME("Performance", "Rendering Time", sRenderingTimeMS);
SORT_STATS_AVG_RAY_SECOND("Performance", "Number of rays per second", sRayCount , sRenderingTimeMS);
SORT_STATS_COUNTER("Statistics", "Sample per Pixel", sSamplePerPixel);
SORT_STATS_COUNTER("Performance", "Worker thread number", sThreadCnt);

// render the image
void System::Render()
{
	// pre-process before rendering
	PreProcess();

    Timer timer;

	// push rendering task
	_pushRenderTask();
	// execute rendering tasks
	_executeRenderingTasks();
	
    SORT_STATS(sRenderingTimeMS = timer.GetElapsedTime());
	SORT_STATS(sThreadCnt = g_threadCnt);
	SORT_STATS(sSamplePerPixel = g_samplePerPixel);
}

// pre-process before rendering
void System::PreProcess()
{
    Timer timer;

	if( m_imagesensor == 0 )
	{
        slog( WARNING , GENERAL , "There is no render target in the system, can't render anything." );
		return;
	}

	// preprocess scene
	m_Scene.PreProcess();

    SORT_STATS(sPreprocessTimeMS = timer.GetElapsedTime());
}

// output progress
void System::_outputProgress()
{
    unsigned progress = 0;
    
    while( progress < 100 ){
        // get the number of tasks done
        unsigned taskDone = 0;
        for( unsigned i = 0; i < m_totalTask; ++i )
            taskDone += m_taskDone[i];
        
        // output progress
        progress = (unsigned)( (float)(taskDone) / (float)m_totalTask * 100 );
        
        if (!g_blenderMode)
            std::cout<< "Progress: "<<progress<<"%\r";
        else if (m_pProgress)
            *m_pProgress = progress;
        
        if( taskDone == m_totalTask )
            break;
    }
}

// uninitialize
void System::Uninit()
{
    // Post process for image sensor
    m_imagesensor->PostProcess();

    // relase the memory
    m_Scene.Release();

    // delete the data
    SAFE_DELETE(m_imagesensor);
    SAFE_DELETE_ARRAY(m_taskDone);
}

// push rendering task
void System::_pushRenderTask()
{
	g_integrator->PreProcess();
	g_integrator->SetupCamera(m_Scene.GetCamera());

	// Push render task into the queue
	const unsigned tilesize = g_tileSize;
	
	// get the number of total task
	m_totalTask = 0;
	for( unsigned i = 0 ; i < m_imagesensor->GetHeight() ; i += tilesize )
		for( unsigned j = 0 ; j < m_imagesensor->GetWidth() ; j += tilesize )
			++m_totalTask;
	m_taskDone = new bool[m_totalTask];
	memset( m_taskDone , 0 , m_totalTask * sizeof(bool) );

	Vector2i tile_num = Vector2i( (int)ceil(m_imagesensor->GetWidth() / (float)tilesize) , (int)ceil(m_imagesensor->GetHeight() / (float)tilesize) );

	// start tile from center instead of top-left corner
	Vector2i cur_pos( tile_num / 2 );
	int cur_dir = 0;
	int cur_len = 0;
	int cur_dir_len = 1;
	const Vector2i dir[4] = { Vector2i( 0 , -1 ) , Vector2i( -1 , 0 ) , Vector2i( 0 , 1 ) , Vector2i( 1 , 0 ) };

	unsigned int priority = DEFAULT_TASK_PRIORITY;
	while (true)
	{
		// only process node inside the image region
		if (cur_pos.x >= 0 && cur_pos.x < tile_num.x && cur_pos.y >= 0 && cur_pos.y < tile_num.y )
		{
			Vector2i size , tl ;
			tl.x = cur_pos.x * tilesize;
			tl.y = cur_pos.y * tilesize;
			size.x = (tilesize < (m_imagesensor->GetWidth() - tl.x)) ? tilesize : (m_imagesensor->GetWidth() - tl.x);
			size.y = (tilesize < (m_imagesensor->GetHeight() - tl.y)) ? tilesize : (m_imagesensor->GetHeight() - tl.y);

			SCHEDULE_TASK<Render_Task>( priority-- , tl , size , &m_Scene , g_samplePerPixel , g_integrator , 
                                        std::make_shared<RandomSampler>() , new PixelSample[g_samplePerPixel] );
		}

		// turn to the next direction
		if (cur_len >= cur_dir_len)
		{
			cur_dir = (cur_dir + 1) % 4;
			cur_len = 0;
			cur_dir_len += 1 - cur_dir % 2;
		}

		cur_pos += dir[cur_dir];

		++cur_len;

		if( (cur_pos.x < 0 || cur_pos.x >= tile_num.x ) && (cur_pos.y < 0 || cur_pos.y >= tile_num.y ) )
			return;
	}
}

// do ray tracing in a multithread enviroment
void System::_executeRenderingTasks()
{
    m_imagesensor->PreProcess();

	// pre allocate memory for the specific thread
	for( unsigned i = 0 ; i <= g_threadCnt ; ++i )
		MemManager::GetSingleton().PreMalloc( 1024 * 1024 * 1024 , i );
    
    std::vector< std::unique_ptr<WorkerThread> > threads;
    for( unsigned i = 0 ; i < g_threadCnt ; ++i )
        threads.push_back( std::unique_ptr<WorkerThread>( new WorkerThread( i + 1 ) ) );

    // start all threads
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->BeginThread(); } );

    // no output progress for now.
	// will need to support low priority thread in the future for such tasks.
    //_outputProgress();

	EXECUTING_TASKS();

	// wait for all the threads to be finished
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<WorkerThread>& thread ) { thread->Join(); } );
}

// setup system from file
bool System::Setup( const char* str )
{
	IFileStream s( str );

	// Load the global configuration from stream
	GlobalConfiguration::GetSingleton().Serialize(s);

	// Load materials from stream
	MatManager::GetSingleton().ParseMatFile( s );

	// setup image sensor first of all
	if( g_blenderMode )
		m_imagesensor = new BlenderImage();
	else
		m_imagesensor = new RenderTargetImage();
	m_imagesensor->SetSensorSize( g_resultResollution.x , g_resultResollution.y );

	// Serialize the scene entities
	m_Scene.LoadScene( s );

	// create shared memory
	int x_tile = (int)(ceil(m_imagesensor->GetWidth() / (float)g_tileSize));
	int y_tile = (int)(ceil(m_imagesensor->GetHeight() / (float)g_tileSize));
	int header_size = x_tile * y_tile;
	int size = header_size * g_tileSize * g_tileSize * 4 * sizeof(float) * 2	// image size
			+ header_size								// header size
			+ 2;										// progress data and final update flag

	// create shared memory
	const SharedMemory& sm = SMManager::GetSingleton().CreateSharedMemory("sharedmem.bin", size, SharedMmeory_All);
	// clear the memory first
	if (sm.bytes)
	{
		// clear the memory
		memset(sm.bytes, 0, sm.size);

		// setup progress pointer
		m_pProgress = sm.bytes + sm.size - 2;
	}

	return true;
}
