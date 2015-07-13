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
#include "utility/path.h"
#include "utility/creator.h"
#include "sampler/sampler.h"
#include "utility/parallel.h"
#include <ImfHeader.h>
#include "utility/strhelper.h"
#include "camera/camera.h"
#include "integrator/integrator.h"
#include "sampler/stratified.h"

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

	// setup default value
	m_rt = 0;
	m_pIntegrator = 0;
	m_camera = 0;
	m_uRenderingTime = 0;
	m_uPreProcessingTime = 0;
	m_uProgressCount = 64;
	m_uCurrentPixelId = 0;
	m_uPreProgress = 0xffffffff;

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
void System::OutputRT()
{
	m_rt->Output( m_strOutputFileName.c_str() );
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
	for( int i = m_rt->GetHeight() -1; i >= 0 ; --i )
	{
		for( unsigned j = 0 ; j < m_rt->GetWidth() ; j++ )
		{
			// clear managed memory after each pixel
			MemManager::GetSingleton().ClearMem();

			// generate samples to be used later
			m_pIntegrator->GenerateSample( m_pSampler , m_pSamples , m_iSamplePerPixel , m_Scene );

			// the radiance
			Spectrum radiance;
			for( unsigned p = 0 ; p < m_camera->GetPassCount() ; ++p )
			{
				for( unsigned k = 0 ; k < m_iSamplePerPixel ; ++k )
				{
					// generate rays
					Ray r = m_camera->GenerateRay( p , (float)j , (float)i , m_pSamples[k] );
					// accumulate the radiance
					radiance += m_pIntegrator->Li( r , m_pSamples[k] ) * m_camera->GetPassFilter(p);
				}
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
			for( unsigned p = 0 ; p < m_camera->GetPassCount() ; ++p )
			{
				for( unsigned k = 0 ; k < m_iSamplePerPixel ; ++k )
				{
					// generate rays
					Ray r = m_camera->GenerateRay( p , (float)j , (float)i , m_pSamples[k] );
					// accumulate the radiance
					radiance += m_pIntegrator->Li( r , m_pSamples[k] ) * m_camera->GetPassFilter(p);
				}
			}
			m_rt->SetColor( j , i , radiance / (float)m_iSamplePerPixel );
			
			{	// update current pixel
				m_uCurrentPixelId++;
			}
		}
		{
			// output progress
			_outputProgress();
		}
	}
	cout<<endl;
}

// setup system from file
bool System::Setup( const char* str )
{
	// load the xml file
	string full_name = GetFullPath(str);
	TiXmlDocument doc( full_name.c_str() );
	doc.LoadFile();
	
	// if there is error , return false
	if( doc.Error() )
	{
		LOG_ERROR<<doc.ErrorDesc()<<CRASH;
		return false;
	}
	
	// get the root of xml
	TiXmlNode*	root = doc.RootElement();
	
	// try to load the scene , note: only the first node matters
	TiXmlElement* element = root->FirstChildElement( "Scene" );
	if( element )
	{
		const char* str_scene = element->Attribute( "value" );
		if( str_scene )
			LoadScene(str_scene);
		else
			return false;
	}else
		return false;
	
	// get the integrater
	element = root->FirstChildElement( "Integrator" );
	if( element )
	{
		const char* str_type = element->Attribute( "type" );
		m_pIntegrator = CREATE_TYPE( str_type , Integrator );
		
		if( m_pIntegrator == 0 )
		{
			LOG_WARNING<<"No integrator with name of "<<str_type<<"."<<ENDL;
			return false;
		}

		// set the properties
		TiXmlElement* prop = element->FirstChildElement( "Property" );
		while( prop )
		{
			const char* prop_name = prop->Attribute( "name" );
			const char* prop_value = prop->Attribute( "value" );
			if( prop_name != 0 && prop_value != 0 )
				m_pIntegrator->SetProperty( prop_name , prop_value );
			prop = prop->NextSiblingElement( "Property" );
		}
	}else
		return false;
	
	// get the render target
	m_rt = new RenderTarget();
	element = root->FirstChildElement( "RenderTargetSize" );
	if( element )
	{
		const char* str_width = element->Attribute("w");
		const char* str_height = element->Attribute("h");
		
		if( str_width && str_height )
		{
			unsigned width = atoi( str_width );
			unsigned height = atoi( str_height );
		
			if( width < 16 ) width = 16;
			if( width > 4096 ) width = 4096;
			if( height < 16 ) height = 16;
			if( height > 3072 ) height = 3072;
		
			m_rt->SetSize( width , height );
		}
	}else
	{
		// use 1024x768 image as default
		m_rt->SetSize(1024, 768);
	}
	
	// get output file name
	element = root->FirstChildElement( "OutputFile" );
	if( element )
	{
		const char* str_name = element->Attribute("name");
		if( str_name )
			m_strOutputFileName = std::string( str_name );
	}
	
	// get sampler
	element = root->FirstChildElement( "Sampler" );
	if( element )
	{
		const char* str_type = element->Attribute("type");
		const char* str_round = element->Attribute("round");
		
		unsigned round = atoi( str_round );
		if( round < 1 ) round = 1;
		if( round > 1024 ) round = 1024;
		
		// create sampler
		m_pSampler = CREATE_TYPE( str_type , Sampler );
		m_iSamplePerPixel = m_pSampler->RoundSize(round);
		m_pSamples = new PixelSample[m_iSamplePerPixel];
	}else{
		// user stratified sampler as default sampler
		m_pSampler = new StratifiedSampler();
		m_iSamplePerPixel = m_pSampler->RoundSize(16);
		m_pSamples = new PixelSample[m_iSamplePerPixel];
	}
	
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
	
	// setup render target
	m_camera->SetRenderTarget(m_rt);
	
	// update total pixel count
	m_uTotalPixelCount = m_rt->GetWidth() * m_rt->GetHeight();
	
	return true;
}