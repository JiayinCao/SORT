/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "entity.h"

//! @brief  Visual entity has a list of visuals.
/**
 * Visual entity is the basic visible element in the virtual world. It has a list
 * of visuals in it. Most of the objects in the world are VisualEntity. Other rare
 * types, like AreaLightEntity, are also a derived class of VisualEntity.
 */
class VisualEntity : public Entity{
public:
    DEFINE_RTTI( VisualEntity , Entity );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the entity. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void    Serialize( IStreamBase& stream ) override {
        stream >> m_transform;

        unsigned int visualCnt = 0;
        stream >> visualCnt;

        while( visualCnt-- > 0 ){
            // Instance the visual
            StringID class_name;
            stream >> class_name;
            auto visual = MakeUniqueInstance<Visual>( class_name );
            visual->Serialize( stream );

            // Apply transform, some Visual applies transformation eariler for better performance.
            visual->ApplyTransform( m_transform );

            m_visuals.push_back( std::move(visual) );
        }
    }
};