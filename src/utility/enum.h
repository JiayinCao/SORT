/*
 * filename :	enum.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_ENUM
#define	SORT_ENUM

// texture output type
enum TEX_TYPE
{
	TT_BMP = 0,
	TT_END ,
};

// mesh file type
enum MESH_TYPE
{
	MT_OBJ = 0,
	MT_PLY = 1,
	MT_END ,
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
	BXDF_SPECULAR = 4,
	BXDF_REFLECTION = 8,
	BXDF_REFRACTION = 16,
	BXDF_ALL_TYPES = BXDF_DIFFUSE | BXDF_GLOSSY | BXDF_SPECULAR ,
	BXDF_ALL_REFLECTION = BXDF_ALL_TYPES | BXDF_REFLECTION ,
	BXDF_ALL_REFRACTION = BXDF_ALL_TYPES | BXDF_REFRACTION ,
	BXDF_ALL = BXDF_ALL_REFLECTION | BXDF_ALL_REFRACTION 
};

#endif
