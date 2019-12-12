/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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
#include "core/define.h"
#include "core/thread.h"

#if defined(SORT_IN_LINUX)
#elif defined(SORT_IN_WINDOWS)
#include <time.h>
#endif

#include <random>

// this is to be totally removed after confirming it works on all platforms.
#define HIDE_OLD_RANDDOM_GENERATOR

#ifndef HIDE_OLD_RANDDOM_GENERATOR
// Random Number Method Definitions
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

// variables used for random number generation
static const int N = 624;
static thread_local unsigned long mt[N]; /* the array for the state vector  */
static thread_local int mti;
static thread_local bool seed_setup = false;

#else

static thread_local std::default_random_engine  re;

static thread_local std::uniform_real_distribution<unsigned>    dist_uint;
static thread_local std::uniform_real_distribution<float>       dist_float( 0.0f , 1.0f );

#endif

// set the seed
void sort_seed()
{
    unsigned _seed = ( ThreadId() + 1 ) * (unsigned)time(0);

#ifndef HIDE_OLD_RANDDOM_GENERATOR
    mt[0]= _seed & 0xffffffffUL;
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
#else
    re.seed( _seed );
#endif
}

// generate a unsigned integer
unsigned sort_rand()
{
#ifndef HIDE_OLD_RANDDOM_GENERATOR
    unsigned long y;
    {
        static thread_local unsigned long mag01[2]={0x0UL, MATRIX_A};
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
#else
    return dist_uint(re);
#endif
}

// generate a canonical random number
float sort_canonical(){
#ifndef HIDE_OLD_RANDDOM_GENERATOR
    return (sort_rand() & 0xffffff) / float(1 << 24);
#else
    return dist_float(re);
#endif
}
