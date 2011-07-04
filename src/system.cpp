/*
 * filename :	System.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "system.h"
#include "managers/texmanager.h"
#include "managers/logmanager.h"
#include "managers/meshmanager.h"
#include "utility/timer.h"
#include "camera/camera.h"
#include "texture/rendertarget.h"
#include "geometry/intersection.h"
#include "integrator/whittedrt.h"
#include "camera/perspective.h"

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
	LogManager::CreateLogManager();
	// initialize texture manager
	TexManager::CreateTexManager();
	// initialize the mesh manager
	MeshManager::CreateMeshManager();
	// initialize the timer
	Timer::CreateTimer();

	// use 800 * 600 render target as default
	m_rt = new RenderTarget();
	m_rt->SetSize( 800 , 600 );
	// there is default value for camera
	float distance = 6.0f;
	PerspectiveCamera* camera = new PerspectiveCamera();
	camera->SetEye( Point( distance , distance , distance ) );
	camera->SetUp( Vector( 0 , 1 , 0 ) );
	camera->SetTarget( Point( 0 , 1 , 0 ) );
	camera->SetFov( 3.1415f / 4 );
	camera->SetRenderTarget( m_rt );
	m_camera = camera;
	// the integrator
	m_pIntegrator = new WhittedRT();

	// set default value
	m_uRenderingTime = 0;
	m_uPreProcessingTime = 0;
	m_uPostProcessingTime = 0;
	m_uProgressCount = 64;
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;
	m_uTotalPixelCount = m_rt->GetWidth() * m_rt->GetHeight();
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

	// release managers
	TexManager::DeleteSingleton();
	MeshManager::DeleteSingleton();
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
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;
	for( unsigned i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// generate rays
			Ray r = m_camera->GenerateRay( j , i );

			Spectrum color = m_pIntegrator->Li( m_Scene , r );
			m_rt->SetColor( j , i , color );

			// update current pixel
			m_uCurrentPixelId++;
		}
		// output progress
		_outputProgress();
	}
	cout<<endl;

	// stop timer
	Timer::GetSingleton().StopTimer();
	m_uRenderingTime = Timer::GetSingleton().GetElapsedTime();

	// post process after rendering
	PostProcess();
}

// output render target
void System::OutputRT( const char* str )
{
	m_rt->Output( str );
}

// load the scene
bool System::LoadScene( const string& str )
{
	return m_Scene.LoadScene( str );
}

// pre-process before rendering
void System::PreProcess()
{
	// set timer before pre-processing
	Timer::GetSingleton().StartTimer();

	if( m_rt == 0 )
		LOG_WARNING<<"There is no render target in the system, can't render anything."<<ENDL;
	if( m_camera == 0 )
		LOG_WARNING<<"There is no camera attached in the system , can't render anything."<<ENDL;
	if( m_pIntegrator == 0 )
		LOG_WARNING<<"There is no integrator attached in the system, can't rendering anything."<<ENDL;

	m_Scene.PreProcess();

	if( m_pIntegrator )
		m_pIntegrator->PreProcess();

	// stop timer
	Timer::GetSingleton().StopTimer();
	m_uPreProcessingTime = Timer::GetSingleton().GetElapsedTime();
}

// post-process after rendering
void System::PostProcess()
{
	// set timer before post-processing
	Timer::GetSingleton().StartTimer();

	m_Scene.PostProcess();

	if( m_pIntegrator )
		m_pIntegrator->PostProcess();

	// stop timer
	Timer::GetSingleton().StopTimer();
	m_uPostProcessingTime = Timer::GetSingleton().GetElapsedTime();
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
	LOG<<"Time spent on post-processing : "<<m_uPostProcessingTime<<ENDL<<ENDL;
}
