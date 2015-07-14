/*
   FileName:      rand.cpp

   Created Time:  2011-08-04 12:43:17

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "rand.h"
#include "sort.h"

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
__declspec(thread) static unsigned long mt[N]; /* the array for the state vector  */
__declspec(thread) static int mti;
__declspec(thread) static bool seed_setup = false;

// set the seed
void sort_seed()
{
	unsigned seed = (unsigned)time(0);
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
		static unsigned long mag01[2]={0x0UL, MATRIX_A};
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