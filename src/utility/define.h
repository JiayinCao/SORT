/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2017 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_DEFINE
#define	SORT_DEFINE

#include "sort.h"

#define	PI		3.1415926f
#define	TWO_PI	6.2831852f
#define	INV_PI	0.3183099f
#define INV_TWOPI 0.15915494f

// some useful macro
#define SAFE_DELETE(p) { if(p) { delete p; p = 0; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] p; p = 0 ; } }

#if defined(SORT_IN_WINDOWS)
#define Thread_Local __declspec(thread)
#elif defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)
#define Thread_Local __thread
#endif

// math macros
#define saturate(x) max(0.0f,min(1.0f,x))
inline float clamp( float x , float mi , float ma )
{
	if( x > ma ) x = ma;
	if( x < mi ) x = mi;
	return x;
}

#endif
