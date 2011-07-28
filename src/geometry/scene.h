/*
 * filename :	scene.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_SCENE
#define	SORT_SCENE

// include the header file
#include "sort.h"
#include <vector>
#include "trimesh.h"
#include "spectrum/spectrum.h"
#include "thirdparty/tinyxml/tinyxml.h"

// pre-decleration of classes
class Accelerator;
class Sky;
class Light;

////////////////////////////////////////////////////////////////////////////
// definition of scene class
class	Scene
{
// public method
public:
	// default constructor
	Scene(){ _init(); }
	// destructor
	~Scene(){ Release(); }

	// load the scene from script file
	// para 'str' : the name for the file
	// result     : 'true' if parsing is successful
	bool	LoadScene( const string& str );

	// get the intersection between a ray and the scene
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	// note     : if there is no acceleration structure , it will iterator all
	//			  of the triangles which will cost much!
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;

	// evaluate sky
	// para 'r' : the ray
	// result   : the sky spectrum
	Spectrum EvaluateSky( const Ray& r ) const;

	// release the memory of the scene
	void	Release();

	// output log information
	void	OutputLog() const;

	// preprocess
	void	PreProcess();

	// get lights
	const vector<Light*>& GetLights() const
	{return m_lights;}

	// get bounding box of the scene
	const BBox& GetBBox() const;

// private field
private:
	// the buffer for the triangle mesh
	vector<TriMesh*>	m_meshBuf;

	// the triangle buffer for the scene
	vector<Primitive*>	m_triBuf;

	// the light
	vector<Light*>		m_lights;

	// the acceleration structure for the scene
	Accelerator*		m_pAccelerator;

	// the file name for the scene
	string		m_filename;

	// the sky for the scene
	Sky*		m_pSky;

	// bounding box for the scene
	mutable BBox	m_BBox;

// private method
	
	// brute force intersection test ( it will only invoked if there is no acceleration structor
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	bool	_bfIntersect( const Ray& r , Intersection* intersect ) const;

	// generate triangle buffer
	void	_generateTriBuf();

	// initialize default data
	void	_init();

	// parse transformation
	Transform	_parseTransform( const TiXmlElement* node );
};

#endif
