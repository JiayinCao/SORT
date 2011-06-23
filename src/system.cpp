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
	// use 800 * 600 render target as default
	m_rt = new RenderTarget();
	m_rt->SetSize( 128 , 128 );
	// there is no default value for camera , it must be set in the script file
	m_camera = 0;

	// initialize texture manager
	TexManager::CreateTexManager();
	// initialize log manager
	LogManager::CreateLogManager();
	// initialize the mesh manager
	MeshManager::CreateMeshManager();
	// initialize the timer
	Timer::CreateTimer();
}

// post-uninit
void System::_postUninit()
{
	// release managers
	TexManager::DeleteSingleton();
	MeshManager::DeleteSingleton();
	Timer::DeleteSingleton();
	LogManager::DeleteSingleton();

	// delete the data
	SAFE_DELETE( m_rt );
	SAFE_DELETE( m_camera );
}

#include "geometry/trimesh.h"
#include "geometry/bbox.h"

// render the image
void System::Render( TriMesh* mesh )
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

	for( unsigned i = 0 ; i < m_rt->GetHeight() ; i++ )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// generate rays
			Ray r = m_camera->GenerateRay( j , i );

			bool intersect = false;

			int n = mesh->m_pMemory->m_iTriNum;
			for( int k = 0 ; k < n ; k++ )
			{
				if( mesh->m_triBuffer[k]->GetIntersect( r ) > 0 )
				{
					intersect = true;
					break;
				}
			}

			if( intersect )
				m_rt->SetColor( j , i , 1 , 1 , 1 );
			else
				m_rt->SetColor( j , i , 0 , 0 , 0 );
		}
	}
}

// output render target
void System::OutputRT( const char* str )
{
	m_rt->Output( str );
}
