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
	// there is no default value for camera , it must be set in the script file
	m_camera = 0;
}

// post-uninit
void System::_postUninit()
{
	// relase the memory
	m_Scene.Release();

	// delete the data
	SAFE_DELETE( m_rt );
	SAFE_DELETE( m_camera );

	// release managers
	TexManager::DeleteSingleton();
	MeshManager::DeleteSingleton();
	Timer::DeleteSingleton();
	LogManager::DeleteSingleton();
}

// render the image
void System::Render()
{
	// preprocess before rendering
	PreProcess();

	for( unsigned i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// generate rays
			Ray r = m_camera->GenerateRay( j , i );

			Intersection ip;
			if( m_Scene.GetIntersect( r , &ip ) )
				m_rt->SetColor( j , i , fabs( ip.normal.x )+1.0f , fabs( ip.normal.y )+1.0f , fabs( ip.normal.z ) );
			else
				m_rt->SetColor( j , i , 0 , 0 , 0 );
		}
	}

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
}

// post-process after rendering
void System::PostProcess()
{
}
