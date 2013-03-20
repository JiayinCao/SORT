/*
   FileName:      strhelper.h

   Created Time:  2011-08-04 12:43:45

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_STRHELPER
#define	SORT_STRHELPER

#include "sort.h"
#include "enum.h"
#include "geometry/transform.h"
#include "spectrum/spectrum.h"

// pre-decleration
struct VertexIndex;

////////////////////////////////////////////////////////////////////////
// there are some global functions for parsing string

// convert string to vertex index
// para 'str' : the string to parse
// result     : a vertex index
VertexIndex	VertexIndexFromStr( const string& str );

// get the type of file according to the file extension
// para 'str' : string to parse
// result     : a cooresponding mesh type
MESH_TYPE	MeshTypeFromStr( const string& str );

// get the type of image file from file extension
// para 'str' : string to parse
// result     : a cooresponding mesh type
TEX_TYPE	TexTypeFromStr( const string& str );

// get the type of integrator from file extension
// para 'str' : string to parse
// result     : a cooresponding integrator type
INTEGRATOR_TYPE	IntegratorTypeFromStr( const string& str );

// get the sampler type from string
// para 'str' : string to parse
// result     : a cooresponding sampler type
SAMPLER_TYPE SamplerTypeFromStr( const std::string& str );

// get the camera type from string
// para 'str' : string to parse
// result     : a coorespoding sampler type
CAMERA_TYPE CameraTypeFromStr( const std::string& str );

class ToLower{
public:
     char operator()(char val){   
        return tolower( val );
    }
};

// transform from string
Transform TransformFromStr( const string& str );

// spectrum from string
Spectrum SpectrumFromStr( const string& str );

// point from string
Point PointFromStr( const string& str );

// direction from string
Vector VectorFromStr( const string& str );

// get the next token
string NextToken( string& str , char t );

#endif
