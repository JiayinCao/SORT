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

// include the header
#include "scene.h"
#include "geometry/intersection.h"
#include "accel/accelerator.h"
#include "utility/strhelper.h"
#include "utility/path.h"
#include "utility/samplemethod.h"
#include "utility/sassert.h"
#include "managers/matmanager.h"
#include "light/light.h"
#include "shape/shape.h"
#include "utility/sassert.h"
#include "utility/stats.h"

SORT_STATS_DEFINE_COUNTER(sScenePrimitiveCount)
SORT_STATS_DEFINE_COUNTER(sSceneLightCount)

SORT_STATS_COUNTER("Scene", "Total Primitive Count", sScenePrimitiveCount);
SORT_STATS_COUNTER("Scene", "Total Light Count", sSceneLightCount);

// load the scene from script file
bool Scene::LoadScene( const string& str )
{
	// copy the filename
	m_filename = str;

	// load the xml file
	TiXmlDocument doc( str.c_str() );
	doc.LoadFile();

    sAssert( !doc.Error() , GENERAL );

	// get the root of xml
	TiXmlNode*	root = doc.RootElement();

	// get the resource path if there is
	string oldpath = GetResourcePath();
	TiXmlElement* element = root->FirstChildElement( "Resource" );
	if( element )
	{
		const char* path = element->Attribute( "path" );
		if( path )	SetResourcePath( path );
	}

	// parse materials
	TiXmlElement* material = root->FirstChildElement( "Material" );
	while( material )
	{
		const char* mat_name = material->Attribute( "value" );
		if( mat_name != 0 )
			MatManager::GetSingleton().ParseMatFile( mat_name );
		material = material->NextSiblingElement( "Material" );
	}

	// parse the triangle mesh
	TiXmlElement* meshNode = root->FirstChildElement( "Model" );
	while( meshNode )
	{
		// get the name of the file
		const char* filename = meshNode->Attribute( "filename" );

		if( filename != 0 )
		{
			// the name of the model
			const char* model_name = meshNode->Attribute( "name" );
			if( model_name == nullptr ){
                slog( WARNING , GENERAL , stringFormat("Mesh defined in file %s doesn't have a model name, it will be skipped." , filename ) );
				break;
			}
			if( nullptr != GetTriMesh( model_name ) ){
                slog( WARNING , GENERAL , stringFormat("A mesh with name %s already existed." , model_name ) );
				break;
			}

			// load the transform matrix
			Transform transform = _parseTransform( meshNode->FirstChildElement( "Transform" ) );

			// load the first mesh
			TriMesh* mesh = new TriMesh(model_name);
			if( mesh->LoadMesh( filename , transform ) )
			{
				// reset the material if neccessary
				TiXmlElement* meshMat = meshNode->FirstChildElement( "Material" );
				if( meshMat )
				{
					meshMat = meshMat->FirstChildElement( "MatSet" );
					do
					{
						const char* set_name = meshMat->Attribute( "name" );
						const char* mat_name = meshMat->Attribute( "mat" );

						if( set_name != 0 && mat_name != 0 )
							mesh->ResetMaterial( set_name , mat_name );

						meshMat = meshMat->NextSiblingElement( "MatSet" );
					}while( meshMat );
				}
				m_meshBuf.push_back( mesh );
			}
			else
				delete mesh;
		}

		// get to the next model
		meshNode = meshNode->NextSiblingElement( "Model" );
	}
	// generate triangle buffer after parsing from file
	_generateTriBuf();
	
	// parse the lights
	TiXmlElement* lightNode = root->FirstChildElement( "Light" );
	while( lightNode )
	{
		const char* type = lightNode->Attribute( "type" );
		if( type != 0 )
		{
			Light* light = CREATE_TYPE( type , Light );
			if( light )
			{
				// setup 
				light->SetupScene( this );

				// load the transform matrix
				light->SetTransform( _parseTransform( lightNode->FirstChildElement( "Transform" ) ) );

				// set the properties
				TiXmlElement* prop = lightNode->FirstChildElement( "Property" );
				while( prop )
				{
					const char* prop_name = prop->Attribute( "name" );
					const char* prop_value = prop->Attribute( "value" );
					if( prop_name != 0 && prop_value != 0 )
						light->SetProperty( prop_name , prop_value );
					prop = prop->NextSiblingElement( "Property" );
				}

				Shape* shape = light->GetShape();
				if( shape )
				{
					shape->SetID((unsigned)m_triBuf.size() );
					m_triBuf.push_back( shape );
				}

				if( strcmp( type , "skylight" ) == 0 )
					m_skyLight = light;

				m_lights.push_back( light );
			}
		}else
            slog( WARNING , LIGHT , stringFormat( "Undefined light type %s" , type ) );

		// get to the next light
		lightNode = lightNode->NextSiblingElement( "Light" );
	}
	_genLightDistribution();
	
	// get accelerator if there is, if there is no accelerator, intersection test is performed in a brute force way.
	TiXmlElement* accelNode = root->FirstChildElement( "Accel" );
	if( accelNode )
	{
		// set cooresponding type of accelerator
		const char* type = accelNode->Attribute( "type" );
		if( type != 0 )	m_pAccelerator = CREATE_TYPE( type , Accelerator );
	}

	// restore resource path
	SetResourcePath( oldpath );
    
    SORT_STATS(sScenePrimitiveCount=(long long)m_triBuf.size());
    SORT_STATS(sSceneLightCount=(long long)m_lights.size());

	return true;
}

// get the intersection between a ray and the scene
bool Scene::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	if( intersect )
		intersect->t = FLT_MAX;

	// brute force intersection test if there is no accelerator
	if( m_pAccelerator == 0 )
		return _bfIntersect( r , intersect );

	return m_pAccelerator->GetIntersect( r , intersect );
}

// get the intersection between a ray and the scene in a brute force way
bool Scene::_bfIntersect( const Ray& r , Intersection* intersect ) const
{
	if( intersect ) intersect->t = FLT_MAX;
	int n = (int)m_triBuf.size();
	for( int k = 0 ; k < n ; k++ )
	{
		bool flag = m_triBuf[k]->GetIntersect( r , intersect );
		if( flag && intersect == 0 )
			return true;
	}

	if( intersect == 0 )
		return false;
	return intersect->t < r.m_fMax && ( intersect->primitive != 0 );
}

// release the memory of the scene
void Scene::Release()
{
	SAFE_DELETE( m_pAccelerator );
	SAFE_DELETE( m_pLightsDis );

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

// parse transformation
Transform Scene::_parseTransform( const TiXmlElement* node )
{
	Transform transform;
	if( node )
	{
		node = node->FirstChildElement( "Matrix" );
		while( node )
		{
			const char* trans = node->Attribute( "value" );
			if( trans )	transform = TransformFromStr( trans ) * transform;
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

// compute light cdf
void Scene::_genLightDistribution()
{
	unsigned count = (unsigned)m_lights.size();
	if( count == 0 )
		return ;

	float* pdf = new float[count];
	for( unsigned i = 0 ; i < count ; i++ )
		pdf[i] = m_lights[i]->Power().GetIntensity();

	float total_pdf = 0.0f;
	for( unsigned i = 0 ; i < count ; i++ )
		total_pdf += pdf[i];

	for( unsigned i = 0 ; i < count ; i++ )
		m_lights[i]->SetPickPDF( pdf[i] / total_pdf );

	SAFE_DELETE(m_pLightsDis);
	m_pLightsDis = new Distribution1D( pdf , count );
	delete[] pdf;
}

// get sampled light
const Light* Scene::SampleLight( float u , float* pdf ) const
{
	sAssert( u >= 0.0f && u <= 1.0f , SAMPLING );
	sAssert( m_pLightsDis != 0 , SAMPLING );

	float _pdf;
	int id = m_pLightsDis->SampleDiscrete( u , &_pdf );
	if( id >= 0 && id < (int)m_lights.size() && _pdf != 0.0f )
	{
		if( pdf ) *pdf = _pdf;
		return m_lights[id];
	}
	return 0;
}

// get light sample property
float Scene::LightProperbility( unsigned i ) const
{
	sAssert( m_pLightsDis != 0 , LIGHT );
	return m_pLightsDis->GetProperty( i );
}

// get triangle mesh set with a specific name
TriMesh* Scene::GetTriMesh( const string& name ) const
{
	vector<TriMesh*>::const_iterator it = m_meshBuf.begin();
	while( it != m_meshBuf.end() )
	{
		if( (*it)->m_Name == name )
			return *it;
		it++;
	}
	return 0;
}

// evalute sky
Spectrum Scene::Le( const Ray& ray ) const
{
	if( m_skyLight )
	{
		Spectrum r;
		m_skyLight->Le( ray , 0 , r );
		return r;
	}
	return 0.0f;
}
