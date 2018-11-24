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

// include the header
#include "intersection.h"
#include "light/light.h"
#include "geometry/primitive.h"

// get the emissive
Spectrum Intersection::Le( const Vector& wo , float* directPdfA , float* emissionPdf ) const
{
	if( primitive == 0 ) return 0.0f;
	Light* light = primitive->GetLight();
	if( light ) 
		return light->Le( *this , wo , directPdfA , emissionPdf );
	return 0.0f;
}
