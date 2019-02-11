/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "material_node_socket.h"
#include "material_node.h"
#include "stream/stream.h"
#include "core/sassert.h"

void MaterialNodePropertyColor::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    if( m_fromSocket )
        m_fromSocket->GetMaterialProperty(bsdf, result);
    else
        result = color;
}

void MaterialNodePropertyColor::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
    std::string materialNodeName , materialNodeOutputSocket;
    stream >> materialNodeName >> materialNodeOutputSocket;
    if( materialNodeName.empty() ){
        stream >> color;
    }else{
        if( cache.count(materialNodeName) == 0 ){
            std::string materialNodeType;
            stream >> materialNodeType;
            cache[materialNodeName] = MakeUniqueInstance<MaterialNode>( materialNodeType );
            sAssertMsg( cache[materialNodeName] != nullptr , MATERIAL , "There is no material node type named %s in material %s." , materialNodeType.c_str() , materialNodeName.c_str() );
            cache[materialNodeName]->Serialize( stream , cache );
        }
        cache[materialNodeName]->LinkNode( materialNodeOutputSocket , this );
    }
}

void MaterialNodePropertyFloat::GetMaterialProperty( Bsdf* bsdf , float& result ) {
    if( m_fromSocket )
        m_fromSocket->GetMaterialProperty(bsdf, result);
    else
        result = m_value;
}

void MaterialNodePropertyFloat::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
    std::string materialNodeName , materialNodeOutputSocket;
    stream >> materialNodeName >> materialNodeOutputSocket;
    if( materialNodeName.empty() ){
        stream >> m_value;
    }else{
        if( cache.count( materialNodeName) == 0 ){
            std::string materialNodeType;
            stream >> materialNodeType;
            cache[materialNodeName] = MakeUniqueInstance<MaterialNode>( materialNodeType );
            sAssert( cache[materialNodeName] != nullptr , MATERIAL );
            cache[materialNodeName]->Serialize( stream , cache );
        }
        cache[materialNodeName]->LinkNode( materialNodeOutputSocket , this );
    }
    
}

void MaterialNodePropertyString::GetMaterialProperty( Bsdf* bsdf , std::string& result ) {
    result = m_str;
}

void MaterialNodePropertyString::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
    std::string dummy;
    stream >> dummy;
    stream >> m_str;
}

void MaterialNodePropertyBxdf::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){
    if( m_fromSocket )
        m_fromSocket->UpdateBSDF(bsdf, weight);
}

void MaterialNodePropertyBxdf::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
    std::string materialNodeName , materialNodeOutputSocket;
    stream >> materialNodeName;
    stream >> materialNodeOutputSocket;
    if( !materialNodeName.empty() ){
        if( cache.count( materialNodeName) == 0 ){
            std::string materialNodeType;
            stream >> materialNodeType;
            cache[materialNodeName] = MakeUniqueInstance<MaterialNode>( materialNodeType );
            sAssert( cache[materialNodeName] != nullptr , MATERIAL );
            cache[materialNodeName]->Serialize( stream , cache );
        }

        cache[materialNodeName]->LinkNode( materialNodeOutputSocket , this );
    }
}

void MaterialNodePropertyVector::GetMaterialProperty( Bsdf* bsdf , Vector& result ) {
    if( m_fromSocket )
        m_fromSocket->GetMaterialProperty(bsdf, result);
    else
        result = m_vec;
}

void MaterialNodePropertyVector::Serialize( IStreamBase& stream , MaterialNodeCache& cache ){
    std::string materialNodeName , materialNodeOutputSocket;
    stream >> materialNodeName >> materialNodeOutputSocket;
    if( materialNodeName.empty() ){
        stream >> m_vec;
    }else{
        if( cache.count( materialNodeName) == 0 ){
            std::string materialNodeType , materialNodeOutputSocket;
            stream >> materialNodeType >> materialNodeOutputSocket;
            cache[materialNodeName] = MakeUniqueInstance<MaterialNode>( materialNodeType );
            sAssert( cache[materialNodeName] != nullptr , MATERIAL );
            cache[materialNodeName]->Serialize( stream , cache );
        }

        cache[materialNodeName]->LinkNode( materialNodeOutputSocket , this );
    }
}