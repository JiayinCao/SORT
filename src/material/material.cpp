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

// include header file
#include "material.h"
#include "bsdf/bsdf.h"
#include "managers/memmanager.h"
#include "utility/log.h"

Bsdf* Material::GetBsdf( const Intersection* intersect ) const
{
        Bsdf* bsdf = SORT_MALLOC(Bsdf)( intersect );
	m_root.UpdateBSDF(bsdf);
	return bsdf;
}

// parse material
void Material::ParseMaterial( TiXmlElement* element )
{
	// parse node property
	m_root.ParseProperty( element , &m_root );

	// check validation
	if( !m_root.IsNodeValid() )
                slog( WARNING , MATERIAL , stringFormat( "Material '%s' is not valid , a default material will be used." , m_name.c_str() ) );
	else
	        m_root.PostProcess();
}
