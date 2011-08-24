/*
   FileName:      merl.h

   Created Time:  2011-08-04 12:52:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MERL
#define	SORT_MERL

// include the header file
#include "bxdf.h"

/////////////////////////////////////////////////////////////////////////////////////
//	definition of merl bxdf
//	desc :	Merl is short for Mitsubishi Electric Research Laboratories. They provide
//			some measured brdf on the websize http://www.merl.com/brdf/. Merl class is
//			responsible for loading the brdf file they provided.
//	note :	It's very difficult to sample merl brdf according to the distribution. So
//			default method is adapted for sampling directions , which is very bad for
//			large area light such as sky light, it'll produce a lot of noise with 
//			a certain number of samples.
class Merl : public Bxdf
{
// public method
public:
	// constructor from a filename
	// para 'filename' : the file name for the brdf
	Merl( const string& filename );
	// destructor
	~Merl();

	// whether the data is valid
	bool	IsValid() { return m_data != 0 ; }

	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const;

// private field
private:
	// the brdf data
	double*	m_data;

	static const unsigned	MERL_SAMPLING_RES_THETA_H;
	static const unsigned	MERL_SAMPLING_RES_THETA_D;
	static const unsigned	MERL_SAMPLING_RES_PHI_D;
	static const unsigned	MERL_SAMPLING_COUNT;
	static const double		MERL_RED_SCALE;
	static const double		MERL_GREEN_SCALE;
	static const double		MERL_BLUE_SCALE;

	// initialize default data
	void _init();
	// release data
	void _release();
	// load brdf data from file
	void _loadBrdf( const string& filename );
};

#endif
