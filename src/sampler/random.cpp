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

// include the header file
#include "random.h"
#include "utility/sassert.h"
#include "utility/rand.h"

IMPLEMENT_CREATOR( RandomSampler );

// generate sample in one dimension
void RandomSampler::Generate1D( float* sample , unsigned num , bool accept_uniform ) const
{
	sAssert( sample != 0 , SAMPLING );

	for( unsigned i = 0 ; i < num ; ++i )
		sample[i] = sort_canonical();
}

// generate sample in two dimension
void RandomSampler::Generate2D( float* sample , unsigned num , bool accept_uniform ) const
{
	sAssert( sample != 0 , SAMPLING );

	int count = 2 * num;
	for( int i = 0 ; i < count ; i += 2 )
	{
		sample[i] = sort_canonical();
		sample[i+1] = sort_canonical();
	}
}
