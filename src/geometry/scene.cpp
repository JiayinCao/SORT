/*
 * filename :	scene.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "scene.h"
#include "geometry/intersection.h"
#include "accel/accelerator.h"
#include "accel/unigrid.h"

// initialize default data
void Scene::_init()
{
}

// load the scene from script file
bool Scene::LoadScene( const string& str )
{
	// temporary
	TriMesh* mesh = new TriMesh();
	Transform t0 = Scale( 10.0f );
	if( mesh->LoadMesh( "../res/bunny.ply" , t0 ) )
		m_meshBuf.push_back( mesh );
	else
		delete mesh;

	// create another instance
	mesh = new TriMesh();
	Transform t1 = Translate( Vector( 0 , 1 , 2 ) ) * RotateX( 1.0f );
	if( mesh->LoadMesh( "../res/cube.obj" , t1 ) )
		m_meshBuf.push_back( mesh );
	else
		delete mesh;

	// create another instance
	mesh = new TriMesh();
	Transform t2 = Translate( Vector( 0 , 1 , 0 ) ) * RotateX( 1.0f ) * Translate( Vector( 0 , -1 , -1 ) ) * Scale( 0.3f );
	if( mesh->LoadMesh( "../res/teapot.obj" , t2 ) )
		m_meshBuf.push_back( mesh );
	else
		delete mesh;

	// generate triangle buffer after parsing from file
	_generateTriBuf();

	m_pAccelerator = new UniGrid();
	if( m_pAccelerator )
	{
		m_pAccelerator->SetPrimitives( &m_triBuf );
		m_pAccelerator->Build();
	}

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
