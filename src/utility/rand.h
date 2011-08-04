/*
   FileName:      rand.h

   Created Time:  2011-08-04 12:43:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_RAND
#define	SORT_RAND

/*
description :
	Random number generation method, the default 'rand' function provided by c++ standard library is not so good,
	another random number generation method is adapted here.
*/

// set the seed
void		sort_seed();

// generate a unsigned integer
unsigned	sort_rand();

// generate a canonical random number
float		sort_canonical();

#endif