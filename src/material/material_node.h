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

#pragma once

#include <unordered_set>
#include "material_node_socket.h"
#include "core/rtti.h"

//! @brief  Material node is the base class for node in material editor.
/**
 * A material node is a basic unit in material definition. There are multiple types of material nodes in SORT material system.
 * Each material node may have multiple input and output sockets. A material node is purely a collected of data and sockets, it
 * defines no specific behavior of the node itself.
 */
class MaterialNode{
public:
    //! @brief  Empty virtual destructor.
    virtual ~MaterialNode() = default;

    //! @brief  Link node with input socket.
    //!
    //! If there is no such a name called @param channel, it means there is something wrong in serialization process,
    //! in which case the program will crash right away.
    //!
    //! @param  channel     Name of the output socket to be linked.
    //! @param  inputSocket The input socket to link.
    void    LinkNode( const std::string& channel , MaterialNodeInputSocket* inputSocket );
    
    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    virtual void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight );

    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { 
        return MNPT_NONE; 
    }
    
    //! @brief  Post process of the material.
    //!
    //! Heavy operations like loading textures, loading measured BRDFs will be done here.
    virtual void PostProcess();
    
    //! @brief Whether the node is valid or not.
    //!
    //! @return         'True' if it is a valid node, otherwise 'False'.
    inline bool IsNodeValid() const { 
        return m_node_valid; 
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    virtual void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) {}

protected:
    /**< All input sockets in the material node. */
    std::unordered_set< class MaterialNodeInputSocket* >            m_inputs;
    /**< All output sockets in the material node. */
    std::unordered_map< std::string , MaterialNodeOutputSocket* >   m_outputs;
    
    /**< Whether the node is valid or not. */
    bool m_node_valid = true;
    /**< Whether post processing is done for this node. */
    bool m_post_processed = false;
    
    //! @brief  Auto register for node input socket.
    class MaterialNodeInputSocketRegister{
    public:
        //! @brief  Constructor.
        //!
        //! @param  socket      Material node property.
        //! @param  node        Reference of the node that this material property belongs to.
        MaterialNodeInputSocketRegister( MaterialNodeInputSocket& socket , MaterialNode& node ){
            node.m_inputs.insert( &socket );
        }
    };

    //! @brief  Auto register for node output socket.
    class MaterialNodeOutputSocketRegister{
    public:
        //! @brief  Constructor.
        //!
        //! @param  socket      Material node socket.
        //! @param  name        Name of the output socket.
        //! @param  node        Reference of the node that this material property belongs to.
        MaterialNodeOutputSocketRegister( MaterialNodeOutputSocket& socket , 
                                          const std::string& name , 
                                          MaterialNode& node ){
            node.m_outputs[name] = &socket;
            socket.UpdateOwningNode(&node);
        }
    };
};

//! @brief Material output node.
/**
 * By default, each material will have exactly one output node. For those who doesn't have any OutputNode,
 * it is defined as invalid material, a default material will be used to replace this material.
 */
class OutputNode : public MaterialNode{
public:
    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF( output );
};
