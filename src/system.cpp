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
	camera->SetRenderTarget( m_rt );m_camera = 0;
	m_camera = camera;
	// the integrator
	m_pIntegrator = new WhittedRT();
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
	unsigned progressCount = 64;
	unsigned totalPixel = m_rt->GetHeight() * m_rt->GetWidth();
	unsigned currentPixel = 0;
	unsigned preProgress = 0;
	for( unsigned i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// generate rays
			Ray r = m_camera->GenerateRay( j , i );

			Spectrum color = m_pIntegrator->Li( m_Scene , r );
			m_rt->SetColor( j , i , color );

			// update current pixel
			currentPixel++;
		}

		// output progress
		unsigned progress = (unsigned)( (float)(currentPixel * progressCount) / (float)totalPixel );
		if( preProgress != progress )
		{
			cout<<"Tracing <<";
			unsigned k = 0;
			for( ; k < progress ; k++ )
				cout<<"-";
			for( ; k < progressCount ; k++ )
				cout<<" ";
			cout<<">>\r";
			preProgress = progress;
		}
	}
	cout<<endl;
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

	// load the scene
	m_Scene.LoadScene("");
	m_Scene.PreProcess();

	// output log
	m_Scene.OutputLog();
}

// post-process after rendering
void System::PostProcess()
{
}
