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

#include "sort.h"
#include <vector>
#include "sample.h"
#include "managers/memmanager.h"
#include <algorithm>
#include "utility/creator.h"

/////////////////////////////////////////////////////////////////////////////////
// definitation of the sampler
class	Sampler
{
public:
	// default constructor
	Sampler();
	// destructor
	virtual ~Sampler();

	// round the size for sampler
	// para 'size' : number of samples to be generated
	// result      : the modified number
	// note : samples like jittered sampling needs to overwrite the method
	virtual unsigned RoundSize( unsigned size ) const { return size; }

	// generate sample in one dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate1D( float* sample , unsigned num , bool accept_uniform = false ) const = 0;

	// generate sample in two dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate2D( float* sample , unsigned num , bool accept_uniform = false ) const = 0;
};

// generate shuffle index
inline const unsigned* ShuffleIndex( unsigned size )
{
	unsigned* shuffle = SORT_MALLOC_ARRAY( unsigned , size );
	for( unsigned i = 0 ; i < size ; i++ )
		shuffle[i] = i;
	std::random_shuffle( shuffle , shuffle + size );
	
	return shuffle;
}
