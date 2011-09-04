/*
   FileName:      system.cpp

   Created Time:  2011-08-04 12:41:44

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "system.h"
#include "managers/texmanager.h"
#include "managers/logmanager.h"
#include "managers/meshmanager.h"
#include "managers/matmanager.h"
#include "managers/memmanager.h"
#include "utility/timer.h"
#include "texture/rendertarget.h"
#include "geometry/intersection.h"
#include "camera/dofperspective.h"
#include "utility/path.h"
#include "utility/creator.h"
#include "sampler/sampler.h"
#include "utility/parallel.h"
#include <ImfHeader.h>

#include "camera/camera.h"
#include "camera/environment.h"
#include "camera/ortho.h"

#include "integrator/whittedrt.h"
#include "integrator/direct.h"
#include "integrator/pathtracing.h"
#include "integrator/bidirpath.h"

#include "sampler/stratified.h"
#include "sampler/random.h"
#include "sampler/regular.h"

// constructor
System::System()
{
	_preInit();
}
// destructor
System::~System()
{
	_postUninit();
}

// pre-initialize
void System::_preInit()
{
	// initialize log manager
	LogManager::CreateInstance();
	// initialize texture manager
	TexManager::CreateInstance();
	// initialize mesh manager
	MeshManager::CreateInstance();
	// initialize material manager
	MatManager::CreateInstance();
	// initialize memory manager
	MemManager::CreateInstance();
	// initialize the timer
	Timer::CreateInstance();

	/////////////////////////////////////////////////////////////////////////////////
	// temp
	// use 800 * 600 render target as default
	m_rt = new RenderTarget();
	m_rt->SetSize( 800 , 600 );
	// there is default value for camera
	float distance = 5000.0f;
//	DofPerspective* camera = new DofPerspective();
//	OrthoCamera* camera = new OrthoCamera();
//	EnvironmentCamera* camera = new EnvironmentCamera();
	PerspectiveCamera* camera = new PerspectiveCamera();
	camera->SetEye( Point( 0 , distance * 0.1f , distance ) );
	camera->SetUp( Vector( 0 , 1 , 0 ) );
	camera->SetTarget( Point( 0 , distance * 0.05f , 0 ) );
	camera->SetFov( 3.1415f / 4 );
	camera->SetRenderTarget( m_rt );
//	camera->SetCameraWidth( 1000.0f );
//	camera->SetCameraHeight( 1000.0f );
//	Vector vec( distance , distance * 0.25f , distance );
//	camera->SetFocalDistance( vec.Length() );
//	camera->SetLen( 40.0f );
	m_camera = camera;
	// the integrator
	//m_pIntegrator = new DirectLight( m_Scene , 1 );
	m_pIntegrator = new PathTracing( m_Scene , 1024 );
	//m_pIntegrator = new WhittedRT(m_Scene);
	//m_pIntegrator = new BidirPathTracing( m_Scene , 1024 );
	// the sampler
	m_pSampler = new StratifiedSampler();
	m_iSamplePerPixel = m_pSampler->RoundSize(1);
	m_pSamples = new PixelSample[m_iSamplePerPixel];

	// set default value
	m_uRenderingTime = 0;
	m_uPreProcessingTime = 0;
	m_uProgressCount = 64;
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;
	m_uTotalPixelCount = m_rt->GetWidth() * m_rt->GetHeight();

	if( MultiThreadEnabled() )
		_setupMultiThreads();
}

// post-uninit
void System::_postUninit()
{
	// relase the memory
	m_Scene.Release();

	// delete the data
	SAFE_DELETE( m_rt );
	SAFE_DELETE( m_camera );
	SAFE_DELETE( m_pIntegrator );
	SAFE_DELETE( m_pSampler );
	SAFE_DELETE_ARRAY( m_pSamples );

	// release managers
	Creator::DeleteSingleton();
	MatManager::DeleteSingleton();
	TexManager::DeleteSingleton();
	MeshManager::DeleteSingleton();
	MemManager::DeleteSingleton();
	Timer::DeleteSingleton();
	LogManager::DeleteSingleton();
}

// render the image
void System::Render()
{
	// pre-process before rendering
	PreProcess();

	// set timer before rendering
	Timer::GetSingleton().StartTimer();

	// reset pixel id
	if( MultiThreadEnabled() )
		_raytracing_multithread();
	else
		_raytracing();
	
	// stop timer
	Timer::GetSingleton().StopTimer();
	m_uRenderingTime = Timer::GetSingleton().GetElapsedTime();
}

// output render target
void System::OutputRT( const char* str )
{
	m_rt->Output( str );
}

// load the scene
bool System::LoadScene( const string& filename )
{
	string str = GetFullPath(filename);
	return m_Scene.LoadScene( str );
}

// pre-process before rendering
void System::PreProcess()
{
	// set timer before pre-processing
	Timer::GetSingleton().StartTimer();

	if( m_rt == 0 )
	{
		LOG_WARNING<<"There is no render target in the system, can't render anything."<<ENDL;
		return;
	}
	if( m_camera == 0 )
	{
		LOG_WARNING<<"There is no camera attached in the system , can't render anything."<<ENDL;
		return;
	}
	if( m_pIntegrator == 0 )
	{
		LOG_WARNING<<"There is no integrator attached in the system, can't rendering anything."<<ENDL;
		return;
	}

	// preprocess scene
	m_Scene.PreProcess();

	// preprocess integrator
	m_pIntegrator->RequestSample( m_pSampler , m_pSamples , m_iSamplePerPixel );
	m_pIntegrator->PreProcess();

	// stop timer
	Timer::GetSingleton().StopTimer();
	m_uPreProcessingTime = Timer::GetSingleton().GetElapsedTime();

	// output some information
	_outputPreprocess();
}

// output preprocessing information
void System::_outputPreprocess()
{
	unsigned	core_num = NumSystemCores();
	bool		multi_thread = MultiThreadEnabled();
	cout<<"------------------------------------------------------------------------------"<<endl;
	cout<<" SORT is short for Simple Open-source Ray Tracing."<<endl;
	if( multi_thread )
		cout<<"   Multi-thread is enabled"<<"("<<core_num<<" core"<<((core_num>1)?"s are":" is")<<" detected.)"<<endl;
	else
		cout<<"   Multi-thread is disabled."<<endl;
	cout<<"   "<<m_iSamplePerPixel<<" sample"<<((m_iSamplePerPixel>1)?"s are":" is")<<" used per pixel."<<endl;
	cout<<"   Scene file : "<<m_Scene.GetFileName()<<endl;
	cout<<"   Time spent on preprocessing :"<<m_uPreProcessingTime<<" ms."<<endl;
}

// get elapsed time
unsigned System::GetRenderingTime() const
{
	return m_uRenderingTime;
}

// output progress
void System::_outputProgress()
{
	// output progress
	unsigned progress = (unsigned)( (float)(m_uCurrentPixelId * m_uProgressCount) / (float)m_uTotalPixelCount );

	if( m_uPreProgress == 0xffffffff )
	{
		cout<<"Tracing <<";
		for( unsigned i = 0 ; i < m_uProgressCount ; i++ )
			cout<<" ";
		cout<<" >> \rTracing <<";
	}
	if( m_uPreProgress != progress )
	{
		cout<<"-";
		m_uPreProgress = progress;
		cout.flush();
	}
}

// output log information
void System::OutputLog() const
{
	// output scene information first
	m_Scene.OutputLog();

	// output integrator information
	if( m_pIntegrator )
		m_pIntegrator->OutputLog();

	// output time information
	LOG_HEADER( "Rendering Information" );
	LOG<<"Time spent on pre-processing  : "<<m_uPreProcessingTime<<ENDL;
	LOG<<"Time spent on rendering       : "<<m_uRenderingTime<<ENDL;
}

// uninitialize 3rd party library
void System::_uninit3rdParty()
{
	Imf::staticUninitialize();
}

// uninitialize
void System::Uninit()
{
	_uninit3rdParty();
}

// preprocess mutiple thread
void System::_prepareMemoryForThread()
{
	int core_num = NumSystemCores();
	for( int i = 0 ; i < core_num ; ++i )
		MemManager::GetSingleton().PreMalloc( 1024 * 1024 * 16 , i );
}

// setup multiple threads environment
void System::_setupMultiThreads()
{
	Sort_Assert( MultiThreadEnabled() == true );

	// set thread number
	SetThreadNum();

	// prepare memory
	_prepareMemoryForThread();
}

// do ray tracing
void System::_raytracing()
{
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;
	for( int i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// clear managed memory after each pixel
			MemManager::GetSingleton().ClearMem(ThreadId());

			// generate samples to be used later
			m_pIntegrator->GenerateSample( m_pSampler , m_pSamples , m_iSamplePerPixel , m_Scene );

			// the radiance
			Spectrum radiance;
			for( unsigned k = 0 ; k < m_iSamplePerPixel ; ++k )
			{
				// generate rays
				Ray r = m_camera->GenerateRay( (float)j , (float)i , m_pSamples[k] );
				// accumulate the radiance
				radiance += m_pIntegrator->Li( m_Scene , r , m_pSamples[k] );
			}
			m_rt->SetColor( j , i , radiance / (float)m_iSamplePerPixel );

			// update current pixel
			m_uCurrentPixelId++;
		}
		// output progress
		_outputProgress();
	}
	cout<<endl;
}
// do ray tracing in a multithread enviroment
void System::_raytracing_multithread()
{
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;
	#pragma omp parallel for
	for( int i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// clear managed memory after each pixel
			MemManager::GetSingleton().ClearMem(ThreadId());

			// generate samples to be used later
			m_pIntegrator->GenerateSample( m_pSampler , m_pSamples , m_iSamplePerPixel , m_Scene );

			// the radiance
			Spectrum radiance;
			for( unsigned k = 0 ; k < m_iSamplePerPixel ; ++k )
			{
				// generate rays
				Ray r = m_camera->GenerateRay( (float)j , (float)i , m_pSamples[k] );
				// accumulate the radiance
				radiance += m_pIntegrator->Li( m_Scene , r , m_pSamples[k] );
			}
			m_rt->SetColor( j , i , radiance / (float)m_iSamplePerPixel );
			
			#pragma omp critical
			{	// update current pixel
				m_uCurrentPixelId++;
			}
		}
		// output progress
		_outputProgress();
	}
	cout<<endl;
}
