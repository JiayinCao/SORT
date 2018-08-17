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

#pragma once

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
