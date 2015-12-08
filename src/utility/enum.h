/*
   FileName:      enum.h

   Created Time:  2011-08-04 12:42:49

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_ENUM
#define	SORT_ENUM

// texture output type
enum TEX_TYPE
{
	TT_BMP = 0,
	TT_EXR = 1,
	TT_TGA = 2,
	TT_NONE ,
};

// mesh file type
enum MESH_TYPE
{
	MT_OBJ = 0,
	MT_PLY = 1,
	MT_NONE ,
};

// texture filter
enum TEXCOORDFILTER
{
	TCF_WARP = 0 ,
	TCF_CLAMP ,
	TCF_MIRROR
};

// bxdf type
enum BXDF_TYPE
{
	BXDF_NONE = 0,
	BXDF_DIFFUSE = 1,
	BXDF_GLOSSY = 2,
	BXDF_REFLECTION = 8,
	BXDF_TRANSMISSION = 16,
	BXDF_ALL_TYPES = BXDF_DIFFUSE | BXDF_GLOSSY ,
	BXDF_ALL_REFLECTION = BXDF_ALL_TYPES | BXDF_REFLECTION ,
	BXDF_ALL_TRANSMISSION = BXDF_ALL_TYPES | BXDF_TRANSMISSION ,
	BXDF_ALL = BXDF_ALL_REFLECTION | BXDF_ALL_TRANSMISSION 
};

// camera type
enum CAMERA_TYPE
{
	CT_NONE = 0,
	CT_ORTHO = 1,
	CT_PERSPECTIVE = 2,
	CT_ENVIRONMENT = 3,
};

// integrator type
enum INTEGRATOR_TYPE
{
	IT_WHITTED = 0,
	IT_DIRECT = 1,
	IT_PATHTRACING = 2,
	IT_BDPT = 3,
	IT_NONE
};

// sampler type
enum SAMPLER_TYPE
{
	ST_RANDOM = 0,
	ST_REGULAR = 1,
	ST_STRATIFIED = 2,
	ST_NONE
};

#endif
