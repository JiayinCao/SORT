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
#include "transform.h"
 
// check whether the transform is identity
bool Transform::IdIdentity() const
{
	if( matrix.m[0] != 1.0f || matrix.m[1] != 0.0f || matrix.m[2] != 0.0f || matrix.m[3] != 0.0f )
		return false;
	if( matrix.m[4] != 0.0f || matrix.m[5] != 1.0f || matrix.m[6] != 0.0f || matrix.m[7] != 0.0f )
		return false;
	if( matrix.m[8] != 0.0f || matrix.m[9] != 0.0f || matrix.m[10] != 1.0f || matrix.m[11] != 0.0f )
		return false;
	if( matrix.m[12] != 0.0f || matrix.m[13] != 0.0f || matrix.m[14] != 0.0f || matrix.m[15] != 1.0f )
		return false;

	return true;
}

// whether there is scale factor in the matrix
bool Transform::HasScale() const
{
	return matrix.HasScale();
}
