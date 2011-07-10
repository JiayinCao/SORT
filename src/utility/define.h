/*
 * filename :	define.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_DEFINE
#define	SORT_DEFINE

#define	PI		3.1415926f
#define	INV_PI	0.1383099f

// some useful macro
#define SAFE_DELETE(p) { if(p) { delete p; p = 0; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] p; p = 0 ; } }

// math macros
#define saturate(x) max(0.0f,min(1.0f,x))
inline float clamp( float x , float mi , float ma )
{
	if( x > ma ) x = ma;
	if( x < mi ) x = mi;
	return x;
}

#endif
