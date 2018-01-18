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

#include "regular.h"
#include "utility/sassert.h"
#include <math.h>

IMPLEMENT_CREATOR( RegularSampler );

// generate sample in one dimension
void RegularSampler::Generate1D( float* sample , unsigned num , bool accept_uniform ) const
{
	sAssert( sample != 0 , SAMPLING );

	for( unsigned i = 0 ; i < num ; ++i )
	{
		// fall back to random sampling if uniform sampling is not accepted.
		// For certain cases, like sampling brdf and light, uniform sampling is not acceptable
		if( !accept_uniform )
			sample[i] = sort_canonical();
		else
			sample[i] = ( (float)i + 0.5f ) / (float)num ;
	}
}

// generate sample in two dimension
void RegularSampler::Generate2D( float* sample , unsigned num , bool accept_uniform ) const
{
	sAssert( sample != 0 , SAMPLING );

	unsigned n = (unsigned)sqrt((float)num);
	sAssert( n * n == num , SAMPLING );
	unsigned dn = 2 * n;

	for( unsigned i = 0 ; i < n ; ++i )
	{
		unsigned offset = dn * i;
		for( unsigned j = 0 ; j < dn ; j+=2 )
		{
			// fall back to random sampling if uniform sampling is not accepted.
			// For certain cases, like sampling brdf and light, uniform sampling is not acceptable
			if( !accept_uniform )
			{
				sample[offset+j] = sort_canonical();
				sample[offset+j+1] = sort_canonical();
				continue;
			}

			sample[offset+j] = ( (float)j/2 + 0.5f ) / (float)n ;
			sample[offset+j+1] = ( (float)i + 0.5f ) / (float)n ;
		}
	}
}

// round the size for sampler
unsigned RegularSampler::RoundSize( unsigned size ) const
{
	unsigned r = (unsigned)ceil( sqrt((float)size) );
	return r * r;
}
