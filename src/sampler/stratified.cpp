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

// include the header
#include "stratified.h"
#include "utility/sassert.h"
#include "utility/rand.h"
#include <math.h>

IMPLEMENT_CREATOR( StratifiedSampler );

// generate sample in one dimension
void StratifiedSampler::Generate1D( float* sample , unsigned num , bool accept_uniform ) const
{
	sAssert( sample != 0 , SAMPLING );

	for( unsigned i = 0 ; i < num ; ++i )
		sample[i] = ( (float)i + sort_canonical() ) / (float)num ;
}

// generate sample in two dimension
void StratifiedSampler::Generate2D( float* sample , unsigned num , bool accept_uniform ) const
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
			sample[offset+j] = ( (float)j/2 + sort_canonical() ) / (float)n ;
			sample[offset+j+1] = ( (float)i + sort_canonical() ) / (float)n ;
		}
	}
}

// round the size for sampler
unsigned StratifiedSampler::RoundSize( unsigned size ) const
{
	unsigned r = (unsigned)ceil( sqrt((float)size) );
	return r * r;
}
