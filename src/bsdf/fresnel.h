/*
   FileName:      fresnel.h

   Created Time:  2011-08-04 12:52:27

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_FRESNEL
#define	SORT_FRESNEL

// include the headers
#include "spectrum/spectrum.h"
#include <math.h>

class	Fresnel
{
public:
	// evaluate spectrum
	virtual Spectrum Evaluate( float cosi , float coso) const = 0;
	// evaluate spectrum
	virtual Spectrum Evaluate( float coso ) const
	{
		return Evaluate( coso , coso );
	}
};

class	FresnelNo : public Fresnel
{
public:
	virtual Spectrum Evaluate( float cosi , float coso ) const
	{
		return 1.0f;
	}
};

class	FresnelConductor : public Fresnel
{
// public method
public:
	// constructor
	FresnelConductor( const Spectrum& e , const Spectrum& kk ):
		eta(e) , k(kk)
	{
	}

	/// evaluate spectrum
	virtual Spectrum Evaluate( float cosi , float coso ) const
	{
		float abs_cos = (cosi>0.0f)?cosi:(-cosi);
		float sq_cos = abs_cos * abs_cos;

		Spectrum t = 2 * eta * abs_cos;
		Spectrum tmp_f = eta*eta+k*k;
		Spectrum tmp = tmp_f * sq_cos;
		Spectrum Rparl2 = (tmp - t + 1 ) / ( tmp + t + 1 );
		Spectrum Rperp2 = (tmp_f - t + sq_cos)/(tmp_f + t + sq_cos );

		return (Rparl2+Rperp2)*0.5f;
	}

// private field
private:
	Spectrum eta , k ;
};

class	FresnelDielectric : public Fresnel
{
// public method
public:
	// constructor
	FresnelDielectric( float ei , float et ):
		eta_i(ei) , eta_t(et)
	{
	}

	// evalute spectrum
	virtual Spectrum Evaluate( float cosi , float coso ) const
	{
		float cos_i = ( cosi > 0.0f )?cosi:(-cosi);
		float cos_o = ( coso > 0.0f )?coso:(-coso);
		float t0 = eta_t * cos_i;
		float t1 = eta_i * cos_o;
		float t2 = eta_i * cos_i;
		float t3 = eta_t * cos_o;

		float Rparl = ( t0 - t1 ) / ( t0 + t1 );
		float Rparp = ( t2 - t3 ) / ( t2 + t3 );

		return ( Rparl * Rparl + Rparp * Rparp ) * 0.5f;
	}

	// evaluate spectrum
	virtual Spectrum Evaluate( float coso ) const
	{
		bool enter = coso > 0.0f;
		float ei = eta_i;
		float et = eta_t;
		if( !enter )
		{
			ei = eta_t;
			et = eta_i;
		}

		float eta = ei / et;
		float sini = eta * sqrtf( 1.0f - coso * coso );
		// note , fully reflection is handled in class Reflection
		if( sini >= 1.0f )
			return 1.0f;

		float cosi = sqrtf( 1.0f - sini * sini );
		
		return Evaluate( cosi , coso );
	}

// private field
private:
	float eta_t , eta_i;
};

#endif
