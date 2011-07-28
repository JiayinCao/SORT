/*
 * filename :	scene.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "scene.h"
#include "geometry/intersection.h"
#include "accel/accelerator.h"
#include "utility/strhelper.h"
#include "utility/path.h"
#include "managers/matmanager.h"
#include "sky/skysphere.h"
#include "sky/skybox.h"
#include "light/light.h"

// initialize default data
void Scene::_init()
{
	m_pAccelerator = 0;
	m_pSky = 0;
}

// load the scene from script file
bool Scene::LoadScene( const string& str )
{
	// copy the filename
	m_filename = str;

	// load the xml file
	TiXmlDocument doc( str.c_str() );
	doc.LoadFile();

	// if there is error , return false
	if( doc.Error() )
	{
		LOG_ERROR<<doc.ErrorDesc()<<CRASH;
		return false;
	}

	// get the root of xml
	TiXmlNode*	root = doc.RootElement();

	// get the resource path if there is
	string oldpath = GetResourcePath();
	TiXmlElement* element = root->FirstChildElement( "Resource" );
	if( element )
		SetResourcePath( element->Attribute( "path" ) );

	// parse materials
	TiXmlElement* material = root->FirstChildElement( "Material" );
	while( material )
	{
		MatManager::GetSingleton().ParseMatFile( material->Attribute( "value" ) );
		material = material->NextSiblingElement( "Material" );
	}

	// parse the triangle mesh
	TiXmlElement* meshNode = root->FirstChildElement( "Model" );
	while( meshNode )
	{
		// get the name of the file
		const char* filename = meshNode->Attribute( "filename" );

		// load the transform matrix
		Transform transform = _parseTransform( meshNode->FirstChildElement( "Transform" ) );

		// load the first mesh
		TriMesh* mesh = new TriMesh();
		if( mesh->LoadMesh( filename , transform ) )
			m_meshBuf.push_back( mesh );
		else
			delete mesh;

		// get to the next model
		meshNode = meshNode->NextSiblingElement( "Model" );
	}


	// parse the triangle mesh
	TiXmlElement* lightNode = root->FirstChildElement( "Light" );
	while( lightNode )
	{
		string type = lightNode->Attribute( "type" );
		Light* light = CREATE_TYPE( type , Light );

		if( light )
		{
			// load the transform matrix
			light->SetTransform( _parseTransform( lightNode->FirstChildElement( "Transform" ) ) );

			// set the properties
			TiXmlElement* prop = lightNode->FirstChildElement( "Property" );
			while( prop )
			{
				string prop_name = prop->Attribute( "name" );
				string prop_value = prop->Attribute( "value" );
				light->SetProperty( prop_name , prop_value );
				prop = prop->NextSiblingElement( "Property" );
			}

			m_lights.push_back( light );
		}else
		{
			LOG_WARNING<<"There is no light with the type of \'"<<type<<"\'."<<ENDL;
		}

		// get to the next light
		lightNode = lightNode->NextSiblingElement( "Light" );
	}

	// get accelerator if there is, if there is no accelerator, intersection test is performed in a brute force way.
	TiXmlElement* accelNode = root->FirstChildElement( "Accel" );
	if( accelNode )
	{
		// set cooresponding type of accelerator
		string type = accelNode->Attribute( "type" );
		m_pAccelerator = CREATE_TYPE( type , Accelerator );
	}

	// get accelerator if there is, if there is no accelerator, intersection test is performed in a brute force way.
	TiXmlElement* skyNode = root->FirstChildElement( "Sky" );
	if( skyNode )
	{
		// set cooresponding type of accelerator
		string type = skyNode->Attribute( "type" );
		if( type == "sky_sphere" )
			m_pSky = new SkySphere();
		else if( type == "sky_box" )
			m_pSky = new SkyBox();

		// set the properties
		TiXmlElement* prop = skyNode->FirstChildElement( "Property" );
		while( prop )
		{
			string prop_name = prop->Attribute( "name" );
			string prop_value = prop->Attribute( "value" );
			m_pSky->SetProperty( prop_name , prop_value );
			prop = prop->NextSiblingElement( "Property" );
		}
	}

	// generate triangle buffer after parsing from file
	_generateTriBuf();

	// restore resource path
	SetResourcePath( oldpath );

	return true;
}

// get the intersection between a ray and the scene
bool Scene::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// brute force intersection test if there is no accelerator
	if( m_pAccelerator == 0 )
		return _bfIntersect( r , intersect );

	return m_pAccelerator->GetIntersect( r , intersect );
}

// get the intersection between a ray and the scene in a brute force way
bool Scene::_bfIntersect( const Ray& r , Intersection* intersect ) const
{
	intersect->t = FLT_MAX;
	int n = (int)m_triBuf.size();
	for( int k = 0 ; k < n ; k++ )
		m_triBuf[k]->GetIntersect( r , intersect );

	return intersect->t < r.m_fMax;
}

// release the memory of the scene
void Scene::Release()
{
	SAFE_DELETE( m_pAccelerator );
	SAFE_DELETE( m_pSky );

	vector<Primitive*>::iterator it = m_triBuf.begin();
	while( it != m_triBuf.end() )
	{
		delete *it;
		it++;
	}
	m_triBuf.clear();

	vector<TriMesh*>::iterator tri_it = m_meshBuf.begin();
	while( tri_it != m_meshBuf.end() )
	{
		delete *tri_it;
		tri_it++;
	}
	m_meshBuf.clear();

	vector<Light*>::iterator light_it = m_lights.begin();
	while( light_it != m_lights.end() )
	{
		delete *light_it;
		light_it++;
	}
	m_lights.clear();
}

// generate triangle buffer
void Scene::_generateTriBuf()
{
	// iterator the mesh
	vector<TriMesh*>::iterator it = m_meshBuf.begin();
	while( it != m_meshBuf.end() )
	{
		(*it)->FillTriBuf( m_triBuf );
		it++;
	}
}

// output log information
void Scene::OutputLog() const
{
	LOG_HEADER("Geometry Information");
	LOG<<"Scene File Name:\t"<<m_filename<<ENDL;
	LOG<<"Triangle Count :\t"<<m_triBuf.size()<<ENDL<<ENDL;

	if( m_pAccelerator )
		m_pAccelerator->OutputLog();
}

// preprocess
void Scene::PreProcess()
{
	// set uniform grid as acceleration structure as default
	if( m_pAccelerator )
	{
		m_pAccelerator->SetPrimitives( &m_triBuf );
		m_pAccelerator->Build();
	}
}


// evaluate sky
Spectrum Scene::EvaluateSky( const Ray& r ) const
{
	if( m_pSky )
		return m_pSky->Evaluate( r );

	return 0.0f;
}

// parse transformation
Transform Scene::_parseTransform( const TiXmlElement* node )
{
	Transform transform;
	if( node )
	{
		node = node->FirstChildElement( "Matrix" );
		while( node )
		{
			transform = TransformFromStr(node->Attribute( "value" )) * transform;
			node = node->NextSiblingElement( "Matrix" );
		}
	}
	return transform;
}

// get the bounding box for the scene
const BBox& Scene::GetBBox() const
{
	if( m_pAccelerator != 0 )
		return m_pAccelerator->GetBBox();

	// if there is no bounding box for the scene, generate one
	vector<Primitive*>::const_iterator it = m_triBuf.begin();
	while( it != m_triBuf.end() )
	{
		m_BBox.Union( (*it)->GetBBox() );
		it++;
	}
	return m_BBox;
}
