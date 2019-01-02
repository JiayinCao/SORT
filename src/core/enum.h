/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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