/*
 * filename :	rand.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_RAND
#define	SORT_RAND

/*
description :
	Random number generation method, the default 'rand' function provided by c++ standard library is not so good,
	another random number generation method is adapted here.
	Check "Efficient Jump Ahead for F2-Linear Random Number Generators" for detail.
*/

// set the seed
void		sort_seed();

// generate a unsigned integer
unsigned	sort_rand();

// generate a canonical random number
float		sort_canonical();

#endif