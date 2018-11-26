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
#include "sort.h"
#include "matmanager.h"
#include "utility/path.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "texmanager.h"
#include "texture/constanttexture.h"
#include "utility/creator.h"
#include "bsdf/merl.h"
#include "utility/creator.h"
#include "utility/log.h"
#include "material/material.h"

// find specific material
std::shared_ptr<Material> MatManager::FindMaterial( const std::string& mat_name ) const
{
    std::unordered_map< std::string , std::shared_ptr<Material> >::const_iterator it = m_matPool.find( mat_name );
    return it == m_matPool.end() ? nullptr : it->second;
}

// whether the material exists
std::shared_ptr<Material> MatManager::GetDefaultMat()
{
    static std::shared_ptr<Material> defaultMat = std::make_shared<Material>();
	return defaultMat;
}

// parse material file and add the materials into the manager
unsigned MatManager::ParseMatFile( TiXmlNode* mat_node )
{
	if( !mat_node )
        return 0;
    
	// parse materials
    TiXmlElement* material = mat_node->FirstChildElement( "Material" );
	while( material )
	{
		// parse the material
		std::string name = material->Attribute( "name" );

		// check if there is a material with the specific name
		if( FindMaterial( name ) != 0 )
		{
			// parse the next material
			material = material->NextSiblingElement( "Material" );

			// skip this material
			continue;
		}

        std::shared_ptr<Material> mat = std::make_shared<Material>();
		mat->SetName(name);
		mat->ParseMaterial( material );

		// push the material
		m_matPool.insert( make_pair( name , mat ) );

		// parse the next material
		material = material->NextSiblingElement( "Material" );
	}

	return 0;
}

// get material number
unsigned MatManager::GetMatCount() const
{
	return (unsigned)m_matPool.size();
}
