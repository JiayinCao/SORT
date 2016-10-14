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

#ifndef	SORT_MATERIAL
#define	SORT_MATERIAL

// include the cpp file
#include "utility/propertyset.h"
#include "utility/creator.h"
#include "spectrum/spectrum.h"
#include "utility/referencecount.h"
#include "material_node.h"

// pre-declera classes
class Bsdf;
class Intersection;

///////////////////////////////////////////////////////////
// definition of material
class Material : public ReferenceCount
{
// public method
public:
	Material();
	virtual ~Material();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// set name
	void SetName( const string& n ) { name = n; }
	// get name of the material
	const string& GetName() const { return name; }

	// set root
	MaterialNode* GetRootNode() { return root; }

	// parse material
	void	ParseMaterial( TiXmlElement* element );

// private field
private:
	// the name for the material
	string			name;

	// the root node of the material
	MaterialNode*	root;
};

#endif
