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
#include "system.h"
#include "utility/log.h"
#include "managers/texmanager.h"
#include "managers/meshmanager.h"
#include "managers/matmanager.h"
#include "managers/memmanager.h"
#include "utility/timer.h"
#include "geometry/intersection.h"
#include "utility/path.h"
#include "utility/creator.h"
#include "sampler/sampler.h"
#include "utility/multithread/multithread.h"
#include "utility/strhelper.h"
#include "camera/camera.h"
#include "integrator/integrator.h"
#include "sampler/stratified.h"
#include <time.h>
#include "managers/smmanager.h"
#include "math/vector2.h"
#include "geometry/sky/sky.h"
#include "shape/shape.h"
#include "utility/stats.h"
#include "utility/profile.h"

extern bool g_bBlenderMode;
extern int  g_iTileSize;

SORT_STATS_DEFINE_COUNTER(sPreprocessTime)
SORT_STATS_DEFINE_COUNTER(sRenderingTime)
SORT_STATS_DEFINE_COUNTER(sSamplePerPixel)

SORT_STATS_TIME("Performance", "Pre-processing Time", sPreprocessTime);
SORT_STATS_TIME("Performance", "Rendering Time", sRenderingTime);
SORT_STATS_AVG_RAY_SECOND("Performance", "Number of rays per second", sRayCount , sRenderingTime);
SORT_STATS_COUNTER("Statistics", "Sample per Pixel", sSamplePerPixel);

// render the image
void System::Render()
{
	// pre-process before rendering
	PreProcess();

	// set timer before rendering
	Timer::GetSingleton().StartTimer();

	// push rendering task
	_pushRenderTask();
	// execute rendering tasks
	_executeRenderingTasks();
	
	// stop timer
	auto renderingTime = Timer::GetSingleton().StopTimer();
    SORT_STATS( sRenderingTime = renderingTime );
}

// pre-process before rendering
void System::PreProcess()
{
	// set timer before pre-processing
	Timer::GetSingleton().StartTimer();

	if( m_imagesensor == 0 )
	{
        slog( WARNING , GENERAL , "There is no render target in the system, can't render anything." );
		return;
	}
	if( m_camera == 0 )
	{
        slog( WARNING , GENERAL , "There is no camera attached in the system , can't render anything." );
		return;
	}

	// preprocess scene
	m_Scene.PreProcess();

	// stop timer
	Timer::GetSingleton().StopTimer();
    SORT_STATS(sPreprocessTime = Timer::GetSingleton().GetElapsedTime());
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
        
        if (!g_bBlenderMode)
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
    SAFE_DELETE(m_camera);
    SAFE_DELETE(m_pSampler);
    SAFE_DELETE_ARRAY(m_taskDone);
}

// push rendering task
void System::_pushRenderTask()
{
	// Push render task into the queue
	unsigned tilesize = g_iTileSize;
	unsigned taskid = 0;
	
	// get the number of total task
	m_totalTask = 0;
	for( unsigned i = 0 ; i < m_imagesensor->GetHeight() ; i += tilesize )
		for( unsigned j = 0 ; j < m_imagesensor->GetWidth() ; j += tilesize )
			++m_totalTask;
	m_taskDone = new bool[m_totalTask];
	memset( m_taskDone , 0 , m_totalTask * sizeof(bool) );

	RenderTask rt(m_Scene,m_pSampler,m_camera,m_taskDone,m_iSamplePerPixel);

	//int tile_num_x = ceil(m_imagesensor->GetWidth() / (float)tilesize);
	//int tile_num_y = ceil(m_imagesensor->GetHeight() / (float)tilesize);
	Vector2i tile_num = Vector2i( (int)ceil(m_imagesensor->GetWidth() / (float)tilesize) , (int)ceil(m_imagesensor->GetHeight() / (float)tilesize) );

	// start tile from center instead of top-left corner
	Vector2i cur_pos( tile_num / 2 );
	int cur_dir = 0;
	int cur_len = 0;
	int cur_dir_len = 1;
	const Vector2i dir[4] = { Vector2i( 0 , -1 ) , Vector2i( -1 , 0 ) , Vector2i( 0 , 1 ) , Vector2i( 1 , 0 ) };

	while (true)
	{
		// only process node inside the image region
		if (cur_pos.x >= 0 && cur_pos.x < tile_num.x && cur_pos.y >= 0 && cur_pos.y < tile_num.y )
		{
			rt.taskId = taskid++;
			rt.ori.x = cur_pos.x * tilesize;
			rt.ori.y = cur_pos.y * tilesize;
			rt.size.x = (tilesize < (m_imagesensor->GetWidth() - rt.ori.x)) ? tilesize : (m_imagesensor->GetWidth() - rt.ori.x);
			rt.size.y = (tilesize < (m_imagesensor->GetHeight() - rt.ori.y)) ? tilesize : (m_imagesensor->GetHeight() - rt.ori.y);

			// create new pixel samples
			rt.pixelSamples = new PixelSample[m_iSamplePerPixel];

			// push the render task
            RenderTaskQueue::GetSingleton().PushTask( rt );
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

    std::shared_ptr<Integrator> integrator(_allocateIntegrator());
	integrator->PreProcess();
	integrator->SetupCamera(m_camera);

	// pre allocate memory for the specific thread
	for( unsigned i = 0 ; i < m_thread_num ; ++i )
		MemManager::GetSingleton().PreMalloc( 1024 * 1024 * 1024 , i );
    
    std::vector< std::unique_ptr<PlatformThreadUnit> > threads;
    for( unsigned i = 0 ; i < m_thread_num ; ++i )
        threads.push_back( std::unique_ptr<PlatformThreadUnit>( new PlatformThreadUnit( i , integrator ) ) );

    // start all threads
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<PlatformThreadUnit>& thread ) { thread->BeginThread(); } );

    // output progress
    _outputProgress();
    
	// wait for all the threads to be finished
    for_each( threads.begin() , threads.end() , []( std::unique_ptr<PlatformThreadUnit>& thread ) { thread->Join(); } );
}

// allocate integrator
Integrator*	System::_allocateIntegrator()
{
	Integrator* integrator = CREATE_TYPE( m_integratorType , Integrator );
		
	if( integrator == 0 )
	{
        slog( WARNING , GENERAL , stringFormat( "No integrator with name of %s" , m_integratorType.c_str() ) );
		return 0;
	}

	std::vector<Property>::iterator it = m_integratorProperty.begin();
	while( it != m_integratorProperty.end() )
	{
		integrator->SetProperty(it->_name,it->_property);
		++it;
	}

	return integrator;
}

// setup system from file
bool System::Setup( const char* str )
{
    // setup image sensor first of all
    if( g_bBlenderMode )
        m_imagesensor = new BlenderImage();
    else
        m_imagesensor = new RenderTargetImage();
    
	// load the xml file
	std::string full_name = GetFullPath(str);
	TiXmlDocument doc( full_name.c_str() );
	doc.LoadFile();
	
    sAssertMsg( !doc.Error() , GENERAL , stringFormat( "Can't load scene file %s" , full_name.c_str() ) );
	
	// get the root of xml
	TiXmlNode*	root = doc.RootElement();
	
    // Setup intermediate resource path
    TiXmlElement* element = root->FirstChildElement( "Resource" );
    if( element ){
        const char* path = element->Attribute( "path" );
        if( path )    SetResourcePath( path );
    }
    
	// get the integrater
	element = root->FirstChildElement( "Integrator" );
	if( element )
	{
		m_integratorType = element->Attribute( "type" );

		// set the properties
		TiXmlElement* prop = element->FirstChildElement( "Property" );
		while( prop )
		{
			const char* prop_name = prop->Attribute( "name" );
			const char* prop_value = prop->Attribute( "value" );

			Property property;
			property._name = prop_name;
			property._property = prop_value;
			m_integratorProperty.push_back( property );

			prop = prop->NextSiblingElement( "Property" );
		}
	}else
		return false;
	
	// get the render target
	element = root->FirstChildElement( "RenderTargetSize" );
	if( element )
	{
		const char* str_width = element->Attribute("w");
		const char* str_height = element->Attribute("h");
		m_imagesensor->SetSensorSize( atoi( str_width ) , atoi( str_height ) );
	}else
		m_imagesensor->SetSensorSize( 1920 , 1080 );
	
	// get sampler
	element = root->FirstChildElement( "Sampler" );
	if( element )
	{
		const char* str_type = element->Attribute("type");
		const char* str_round = element->Attribute("round");
		
		unsigned round = atoi( str_round );
		if( round < 1 ) round = 1;
		
		// create sampler
		m_pSampler = CREATE_TYPE( str_type , Sampler );
		m_iSamplePerPixel = m_pSampler->RoundSize(round);
	}else{
		// user stratified sampler as default sampler
		m_pSampler = new StratifiedSampler();
		m_iSamplePerPixel = m_pSampler->RoundSize(16);
	}
    SORT_STATS(sSamplePerPixel = m_iSamplePerPixel);
	
	element = root->FirstChildElement("Camera");
	if( element )
	{
		const char* str_camera = element->Attribute("type");
	
		// create the camera
		m_camera = CREATE_TYPE(str_camera,Camera);
		
		if( !m_camera )
			return false;
		
		// set the properties
		TiXmlElement* prop = element->FirstChildElement( "Property" );
		while( prop )
		{
			const char* prop_name = prop->Attribute( "name" );
			const char* prop_value = prop->Attribute( "value" );
			if( prop_name != 0 && prop_value != 0 )
				m_camera->SetProperty( prop_name , prop_value );
			prop = prop->NextSiblingElement( "Property" );
		}
	}

    element = root->FirstChildElement("Materials");
    MatManager::GetSingleton().ParseMatFile( element );
    
    element = root->FirstChildElement("Scene");
    m_Scene.LoadScene( element );
    
	element = root->FirstChildElement("OutputFile");
	if( element )
        m_imagesensor->SetProperty("filename", element->Attribute("name"));

	element = root->FirstChildElement("ThreadNum");
	if( element )
		m_thread_num = atoi(element->Attribute("name"));
    
	// setup image sensor
    m_camera->SetImageSensor(m_imagesensor);

    // preprocess camera
    m_camera->PreProcess();
    
	// create shared memory
	int x_tile = (int)(ceil(m_imagesensor->GetWidth() / (float)g_iTileSize));
	int y_tile = (int)(ceil(m_imagesensor->GetHeight() / (float)g_iTileSize));
	int header_size = x_tile * y_tile;
	int size = header_size * g_iTileSize * g_iTileSize * 4 * sizeof(float) * 2	// image size
			+ header_size								// header size
			+ 2;										// progress data and final update flag

	// create shared memory
	const SharedMemory& sm = SMManager::GetSingleton().CreateSharedMemory("sharedmem.bin", size, SharedMmeory_All);
	// clear the memory first
	if (sm.bytes)
	{
		// clear the memory
		memset(sm.bytes, 0, sm.size);

		// setup progess pointer
		m_pProgress = sm.bytes + sm.size - 2;
	}

	return true;
}
