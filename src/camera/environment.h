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

#ifndef	SORT_ENVIRONMENT
#define	SORT_ENVIRONMENT

// include header file
#include "camera.h"
#include "math/matrix.h"

//////////////////////////////////////////////////////////////
// definition of environment camera
class	EnvironmentCamera : public Camera
{
// public method
public:
	DEFINE_CREATOR( EnvironmentCamera , "environment" );

	// default constructor
	EnvironmentCamera(){_init();}

	// generate a ray given a pixel
	virtual Ray	GenerateRay( float x , float y , const PixelSample& ps ) const;

	// set transformation matrix
	void SetTransform( const Matrix& m ) { m_transform = m; }

	// get camera coordinate according to a view direction in world space
	virtual Vector2i GetScreenCoord(Point p, float* pdfw, float* pdfa, float* cosAtCamera , Spectrum* we , Point* eyeP , Visibility* visibility) const {
		Sort_Assert(false);
		return Vector2i();
	}

	// get eye direction
	virtual Vector GetForward() const {
		Sort_Assert(false);
		return Vector();
	}

// private field
private:
	// a rotation transformation
	Matrix	m_transform;
	
	// initialize data
	void _init();
	
	// register all properties
	void _registerAllProperty();
};

#endif
