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

#include "strhelper.h"
#include "managers/meshmanager.h"
#include <algorithm>
#include "math/transform.h"
#include <stdarg.h>
#include "core/log.h"

// spectrum from string
Spectrum SpectrumFromStr( const std::string& s )
{
	// if the string is empty , return black color
	if( s.empty() )
		return Spectrum();

	std::string str = s;
	std::string r = NextToken( str , ' ' );
	std::string g = NextToken( str , ' ' );
	std::string b = NextToken( str , ' ' );

	float fr = (float)atof( r.c_str() );
	float fg = (float)atof( g.c_str() );
	float fb = (float)atof( b.c_str() );

	return Spectrum( fr , fg , fb );
}

// get the next token
std::string NextToken( std::string& str , char t )
{
	// get the next t index
	unsigned id = (int)str.find_first_of( t );
	while( id == 0 )
	{
		// get to the next one
		str = str.substr( 1 , std::string::npos );
		id = (int)str.find_first_of( t );
	}
	std::string res = str.substr( 0 , id );

	// if there is no such a character , set it none
	if( id == (unsigned)std::string::npos )
		str = "";
	else
		str = str.substr( id + 1 , std::string::npos );

	return res;
}