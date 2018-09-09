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

#include "rand.h"
#include "sort.h"
#include "utility/define.h"

#if defined(SORT_IN_LINUX) 
#elif defined(SORT_IN_WINDOWS)
#include <time.h>
#endif

// Random Number Method Definitions
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

// variables used for random number generation
static const int N = 624;
static Thread_Local unsigned long mt[N]; /* the array for the state vector  */
static Thread_Local int mti;
static Thread_Local bool seed_setup = false;

#include "utility/multithread/multithread.h"

// set the seed
void sort_seed()
{
	unsigned seed = ( ThreadId() + 1 ) * (unsigned)time(0);
	mt[0]= seed & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }

	seed_setup = true;
}

// generate a unsigned integer
unsigned sort_rand()
{
	unsigned long y;
	{
		static Thread_Local unsigned long mag01[2]={0x0UL, MATRIX_A};
		/* mag01[x] = x * MATRIX_A  for x=0,1 */

		if( seed_setup == false )
			sort_seed();

		if (mti >= N) { /* generate N words at one time */
			int kk;

			if (mti == N+1)   /* if Seed() has not been called, */
				sort_seed(); /* default initial seed */

			for (kk=0;kk<N-M;kk++) {
				y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
				mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			for (;kk<N-1;kk++) {
				y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
				mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
			mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

			mti = 0;
		}

		y = mt[mti++];

		/* Tempering */
		y ^= (y >> 11);
		y ^= (y << 7) & 0x9d2c5680UL;
		y ^= (y << 15) & 0xefc60000UL;
		y ^= (y >> 18);
	}
    return y;
}

// generate a canonical random number
float sort_canonical()
{
	return (sort_rand() & 0xffffff) / float(1 << 24);
}
