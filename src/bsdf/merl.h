/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
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
	Merl();
	// destructor
	~Merl();

	// Load data from file
	void	LoadData( const string& filename );

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
	double*	m_data = nullptr;
    
	// load brdf data from file
	void _loadBrdf( const string& filename );
};

#endif
